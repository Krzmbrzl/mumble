// Copyright 2005-2020 The Mumble Developers. All rights reserved.
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file at the root of the
// Mumble source tree or at <https://www.mumble.info/LICENSE>.

// <cmath> includes <math.h>, but only if it isn't already included.
// We include <cmath> as first header to make sure that we include <math.h> with _USE_MATH_DEFINES.
#ifdef _MSC_VER
# define _USE_MATH_DEFINES
#endif

#include <cmath>

#include "AudioOutput.h"

#include "AudioInput.h"
#include "AudioOutputSample.h"
#include "AudioOutputSpeech.h"
#include "User.h"
#include "Message.h"
#include "PluginManager.h"
#include "PacketDataStream.h"
#include "ServerHandler.h"
#include "Timer.h"
#include "Utils.h"
#include "VoiceRecorder.h"
#include "Channel.h"
#include "ChannelListener.h"
#include "SpeechFlags.h"

// We define a global macro called 'g'. This can lead to issues when included code uses 'g' as a type or parameter name (like protobuf 3.7 does). As such, for now, we have to make this our last include.
#include "Global.h"

// Remember that we cannot use static member classes that are not pointers, as the constructor
// for AudioOutputRegistrar() might be called before they are initialized, as the constructor
// is called from global initialization.
// Hence, we allocate upon first call.

QMap<QString, AudioOutputRegistrar *> *AudioOutputRegistrar::qmNew;
QString AudioOutputRegistrar::current = QString();

AudioOutputRegistrar::AudioOutputRegistrar(const QString &n, int p) : name(n), priority(p) {
	if (! qmNew)
		qmNew = new QMap<QString, AudioOutputRegistrar *>();
	qmNew->insert(name,this);
}

AudioOutputRegistrar::~AudioOutputRegistrar() {
	qmNew->remove(name);
}

AudioOutputPtr AudioOutputRegistrar::newFromChoice(QString choice) {
	if (! qmNew)
		return AudioOutputPtr();

	if (!choice.isEmpty() && qmNew->contains(choice)) {
		g.s.qsAudioOutput = choice;
		current = choice;
		return AudioOutputPtr(qmNew->value(choice)->create());
	}
	choice = g.s.qsAudioOutput;
	if (qmNew->contains(choice)) {
		current = choice;
		return AudioOutputPtr(qmNew->value(choice)->create());
	}

	AudioOutputRegistrar *r = NULL;
	foreach(AudioOutputRegistrar *aor, *qmNew)
		if (!r || (aor->priority > r->priority))
			r = aor;
	if (r) {
		current = r->name;
		return AudioOutputPtr(r->create());
	}
	return AudioOutputPtr();
}

bool AudioOutputRegistrar::canMuteOthers() const {
	return false;
}

bool AudioOutputRegistrar::usesOutputDelay() const {
	return true;
}

bool AudioOutputRegistrar::canExclusive() const {
	return false;
}

AudioOutput::AudioOutput()
    : fSpeakers(NULL)
    , fSpeakerVolume(NULL)
    , bSpeakerPositional(NULL)
    
    , eSampleFormat(SampleFloat)
    
    , bRunning(true)
    
    , iFrameSize(SAMPLE_RATE / 100)
    , iMixerFreq(0)
    , iChannels(0)
    , iSampleSize(0)
    
    , qrwlOutputs()
    , qmOutputs() {
	
	// Nothing
}

AudioOutput::~AudioOutput() {
	bRunning = false;
	wait();
	wipe();

	delete [] fSpeakers;
	delete [] fSpeakerVolume;
	delete [] bSpeakerPositional;
}

// Here's the theory.
// We support sound "bloom"ing. That is, if sound comes directly from the left, if it is sufficiently
// close, we'll hear it full intensity from the left side, and "bloom" intensity from the right side.

float AudioOutput::calcGain(float dotproduct, float distance) {

	float dotfactor = (dotproduct + 1.0f) / 2.0f;
	float att;


	// No distance attenuation
	if (g.s.fAudioMaxDistVolume > 0.99f) {
		att = qMin(1.0f, dotfactor + g.s.fAudioBloom);
	} else if (distance < g.s.fAudioMinDistance) {
		float bloomfac = g.s.fAudioBloom * (1.0f - distance/g.s.fAudioMinDistance);

		att = qMin(1.0f, bloomfac + dotfactor);
	} else {
		float datt;

		if (distance >= g.s.fAudioMaxDistance) {
			datt = g.s.fAudioMaxDistVolume;
		} else {
			float mvol = g.s.fAudioMaxDistVolume;
			if (mvol < 0.01f)
				mvol = 0.01f;

			float drel = (distance-g.s.fAudioMinDistance) / (g.s.fAudioMaxDistance - g.s.fAudioMinDistance);
			datt = powf(10.0f, log10f(mvol) * drel);
		}

		att = datt * dotfactor;
	}
	return att;
}

void AudioOutput::wipe() {
	foreach(AudioOutputUser *aop, qmOutputs)
		removeBuffer(aop);
}

const float *AudioOutput::getSpeakerPos(unsigned int &speakers) {
	if ((iChannels > 0) && fSpeakers) {
		speakers = iChannels;
		return fSpeakers;
	}
	return NULL;
}

void AudioOutput::addFrameToBuffer(ClientUser *user, const QByteArray &qbaPacket, unsigned int iSeq, MessageHandler::UDPMessageType type) {
	if (iChannels == 0)
		return;
	qrwlOutputs.lockForRead();
	AudioOutputSpeech *aop = qobject_cast<AudioOutputSpeech *>(qmOutputs.value(user));

	if (!UDPMessageTypeIsValidVoicePacket(type)) {
		qWarning("AudioOutput: ignored frame with invalid message type 0x%x in addFrameToBuffer().", static_cast<unsigned char>(type));
		return;
	}

	if (! aop || (aop->umtType != type)) {
		qrwlOutputs.unlock();

		if (aop)
			removeBuffer(aop);

		while ((iMixerFreq == 0) && isAlive()) {
			QThread::yieldCurrentThread();
		}

		if (! iMixerFreq)
			return;

		qrwlOutputs.lockForWrite();
		aop = new AudioOutputSpeech(user, iMixerFreq, type);
		qmOutputs.replace(user, aop);
	}

	aop->addFrameToBuffer(qbaPacket, iSeq);

	qrwlOutputs.unlock();
}

void AudioOutput::removeBuffer(const ClientUser *user) {
	removeBuffer(qmOutputs.value(user));
}

void AudioOutput::removeBuffer(AudioOutputUser *aop) {
	QWriteLocker locker(&qrwlOutputs);
	QMultiHash<const ClientUser *, AudioOutputUser *>::iterator i;
	for (i=qmOutputs.begin(); i != qmOutputs.end(); ++i) {
		if (i.value() == aop) {
			qmOutputs.erase(i);
			delete aop;
			break;
		}
	}
}

AudioOutputSample *AudioOutput::playSample(const QString &filename, bool loop) {
	SoundFile *handle = AudioOutputSample::loadSndfile(filename);
	if (handle == NULL)
		return NULL;

	Timer t;
	const quint64 oneSecond = 1000000;

	while (!t.isElapsed(oneSecond) && (iMixerFreq == 0) && isAlive()) {
		QThread::yieldCurrentThread();
	}

	// If we've waited for more than one second, we declare timeout.
	if (t.isElapsed(oneSecond)) {
		qWarning("AudioOutput: playSample() timed out after 1 second: device not ready");
		return NULL;
	}

	if (! iMixerFreq)
		return NULL;

	QWriteLocker locker(&qrwlOutputs);
	AudioOutputSample *aos = new AudioOutputSample(filename, handle, loop, iMixerFreq);
	qmOutputs.insert(NULL, aos);

	return aos;

}

void AudioOutput::initializeMixer(const unsigned int *chanmasks, bool forceheadphone) {
	delete[] fSpeakers;
	delete[] bSpeakerPositional;
	delete[] fSpeakerVolume;

	fSpeakers = new float[iChannels * 3];
	bSpeakerPositional = new bool[iChannels];
	fSpeakerVolume = new float[iChannels];

	memset(fSpeakers, 0, sizeof(float) * iChannels * 3);
	memset(bSpeakerPositional, 0, sizeof(bool) * iChannels);

	for (unsigned int i=0;i<iChannels;++i)
		fSpeakerVolume[i] = 1.0f;

	if (g.s.bPositionalAudio && (iChannels > 1)) {
		for (unsigned int i=0;i<iChannels;i++) {
			float *s = &fSpeakers[3*i];
			bSpeakerPositional[i] = true;

			switch (chanmasks[i]) {
				case SPEAKER_FRONT_LEFT:
					s[0] = -0.5f;
					s[2] = 1.0f;
					break;
				case SPEAKER_FRONT_RIGHT:
					s[0] = 0.5f;
					s[2] = 1.0f;
					break;
				case SPEAKER_FRONT_CENTER:
					s[2] = 1.0f;
					break;
				case SPEAKER_LOW_FREQUENCY:
					break;
				case SPEAKER_BACK_LEFT:
					s[0] = -0.5f;
					s[2] = -1.0f;
					break;
				case SPEAKER_BACK_RIGHT:
					s[0] = 0.5f;
					s[2] = -1.0f;
					break;
				case SPEAKER_FRONT_LEFT_OF_CENTER:
					s[0] = -0.25;
					s[2] = 1.0f;
					break;
				case SPEAKER_FRONT_RIGHT_OF_CENTER:
					s[0] = 0.25;
					s[2] = 1.0f;
					break;
				case SPEAKER_BACK_CENTER:
					s[2] = -1.0f;
					break;
				case SPEAKER_SIDE_LEFT:
					s[0] = -1.0f;
					break;
				case SPEAKER_SIDE_RIGHT:
					s[0] = 1.0f;
					break;
				case SPEAKER_TOP_CENTER:
					s[1] = 1.0f;
					s[2] = 1.0f;
					break;
				case SPEAKER_TOP_FRONT_LEFT:
					s[0] = -0.5f;
					s[1] = 1.0f;
					s[2] = 1.0f;
					break;
				case SPEAKER_TOP_FRONT_CENTER:
					s[1] = 1.0f;
					s[2] = 1.0f;
					break;
				case SPEAKER_TOP_FRONT_RIGHT:
					s[0] = 0.5f;
					s[1] = 1.0f;
					s[2] = 1.0f;
					break;
				case SPEAKER_TOP_BACK_LEFT:
					s[0] = -0.5f;
					s[1] = 1.0f;
					s[2] = -1.0f;
					break;
				case SPEAKER_TOP_BACK_CENTER:
					s[1] = 1.0f;
					s[2] = -1.0f;
					break;
				case SPEAKER_TOP_BACK_RIGHT:
					s[0] = 0.5f;
					s[1] = 1.0f;
					s[2] = -1.0f;
					break;
				default:
					bSpeakerPositional[i] = false;
					fSpeakerVolume[i] = 0.0f;
					qWarning("AudioOutput: Unknown speaker %d: %08x", i, chanmasks[i]);
					break;
			}
			if (g.s.bPositionalHeadphone || forceheadphone) {
				s[1] = 0.0f;
				s[2] = 0.0f;
				if (s[0] == 0.0f)
					fSpeakerVolume[i] = 0.0f;
			}
		}
		for (unsigned int i=0;i<iChannels;i++) {
			float d = sqrtf(fSpeakers[3*i+0]*fSpeakers[3*i+0] + fSpeakers[3*i+1]*fSpeakers[3*i+1] + fSpeakers[3*i+2]*fSpeakers[3*i+2]);
			if (d > 0.0f) {
				fSpeakers[3*i+0] /= d;
				fSpeakers[3*i+1] /= d;
				fSpeakers[3*i+2] /= d;
			}
		}
	}
	iSampleSize = static_cast<int>(iChannels * ((eSampleFormat == SampleFloat) ? sizeof(float) : sizeof(short)));
	qWarning("AudioOutput: Initialized %d channel %d hz mixer", iChannels, iMixerFreq);
}

bool AudioOutput::mix(void *outbuff, unsigned int nsamp) {
	// A list of users that have audio to contribute
	QList<AudioOutputUser *> qlMix;
	// A list of users that no longer have any audio to play and can thus be deleted
	QList<AudioOutputUser *> qlDel;
	
	if (g.s.fVolume < 0.01f) {
		return false;
	}

	const float adjustFactor = std::pow(10.f , -18.f / 20);
	const float mul = g.s.fVolume;
	const unsigned int nchan = iChannels;
	ServerHandlerPtr sh = g.sh;
	VoiceRecorderPtr recorder;
	if (sh) {
		recorder = g.sh->recorder;
	}

	qrwlOutputs.lockForRead();
	
	bool prioritySpeakerActive = false;
	
	// Get the users that are currently talking (and are thus serving as an audio source)
	QMultiHash<const ClientUser *, AudioOutputUser *>::const_iterator it = qmOutputs.constBegin();
	while (it != qmOutputs.constEnd()) {
		AudioOutputUser *aop = it.value();
		if (! aop->prepareSampleBuffer(nsamp)) {
			qlDel.append(aop);
		} else {
			qlMix.append(aop);
			
			const ClientUser *user = it.key();
			if (user && user->bPrioritySpeaker) {
				prioritySpeakerActive = true;
			}
		}
		++it;
	}

	if (g.prioritySpeakerActiveOverride) {
		prioritySpeakerActive = true;
	}

	if (! qlMix.isEmpty()) {
		// There are audio sources available -> mix those sources together and feed them into the audio backend
		STACKVAR(float, speaker, iChannels*3);
		STACKVAR(float, svol, iChannels);

		STACKVAR(float, fOutput, iChannels * nsamp);

		// If the audio backend uses a float-array we can sample and mix the audio sources directly into the output. Otherwise we'll have to
		// use an intermediate buffer which we will convert to an array of shorts later
		float *output = (eSampleFormat == SampleFloat) ? reinterpret_cast<float *>(outbuff) : fOutput;
		bool validListener = false;

		memset(output, 0, sizeof(float) * nsamp * iChannels);

		// Initialize recorder if recording is enabled
		boost::shared_array<float> recbuff;
		if (recorder) {
			recbuff = boost::shared_array<float>(new float[nsamp]);
			memset(recbuff.get(), 0, sizeof(float) * nsamp);
			recorder->prepareBufferAdds();
		}

		for (unsigned int i=0;i<iChannels;++i)
			svol[i] = mul * fSpeakerVolume[i];

		if (g.s.bPositionalAudio && (iChannels > 1) && g.pluginManager->fetchPositionalData()) {
			// Calculate the positional audio effects if it is enabled

			Vector3D cameraDir = g.pluginManager->getPositionalData().getCameraDir();

			Vector3D cameraAxis = g.pluginManager->getPositionalData().getCameraAxis();

			// Front vector is dominant; if it's zero we presume all is zero.

			if (!cameraDir.isZero()) {
				cameraDir.normalize();

				if (!cameraAxis.isZero()) {
					cameraAxis.normalize();
				} else {
					cameraAxis = { 0.0f, 1.0f, 0.0f };
				}

				if (std::abs(cameraDir.dotProduct(cameraAxis)) > 0.01f) {
					// Not perpendicular. Assume Y up and rotate 90 degrees.

					float azimuth = 0.0f;
					if (cameraDir.x != 0.0f || cameraDir.z != 0.0f) {
						azimuth = atan2f(cameraDir.z, cameraDir.x);
					}

					float inclination = acosf(cameraDir.y - static_cast<float>(M_PI) / 2.0f);

					cameraDir.x = sinf(inclination) * cosf(azimuth);
					cameraDir.y = cosf(inclination);
					cameraDir.z = sinf(inclination) * sinf(azimuth);
				}
			} else {
				cameraDir = { 0.0f, 0.0f, 1.0f };

				cameraAxis = { 0.0f, 1.0f, 0.0f };
			}

			// Calculate right vector as front X top
			Vector3D right = cameraDir.crossProduct(cameraAxis);

			/*
						qWarning("Front: %f %f %f", front[0], front[1], front[2]);
						qWarning("Top: %f %f %f", top[0], top[1], top[2]);
						qWarning("Right: %f %f %f", right[0], right[1], right[2]);
			*/
			// Rotate speakers to match orientation
			for (unsigned int i=0;i<iChannels;++i) {
				speaker[3*i+0] = fSpeakers[3*i+0] * right.x + fSpeakers[3*i+1] * cameraAxis.x + fSpeakers[3*i+2] * cameraDir.x;
				speaker[3*i+1] = fSpeakers[3*i+0] * right.y + fSpeakers[3*i+1] * cameraAxis.y + fSpeakers[3*i+2] * cameraDir.y;
				speaker[3*i+2] = fSpeakers[3*i+0] * right.z + fSpeakers[3*i+1] * cameraAxis.z + fSpeakers[3*i+2] * cameraDir.z;
			}
			validListener = true;
		}

		foreach(AudioOutputUser *aop, qlMix) {
			// Iterate through all audio sources and mix them together into the output (or the intermediate array)
			float * RESTRICT pfBuffer = aop->pfBuffer;
			float volumeAdjustment = 1;

			// Check if the audio source is a user speaking (instead of a sample playback) and apply potential volume adjustments
			AudioOutputSpeech *speech = qobject_cast<AudioOutputSpeech *>(aop);
			const ClientUser *user = nullptr;
			if (speech) {
				user = speech->p;
				volumeAdjustment *= user->fLocalVolume;

				if (user->cChannel && ChannelListener::isListening(g.uiSession, user->cChannel->iId) && (speech->ucFlags & SpeechFlags::Listen)) {
					// We are receiving this audio packet only because we are listening to the channel
					// the speaking user is in. Thus we receive the audio via our "listener proxy".
					// Thus we'll apply the volume adjustment for our listener proxy as well
					volumeAdjustment *= ChannelListener::getListenerLocalVolumeAdjustment(user->cChannel);
				}

				if (prioritySpeakerActive) {
					
					if (user->tsState != Settings::Whispering
					    && !user->bPrioritySpeaker) {
						
						volumeAdjustment *= adjustFactor;
					}
				}
			}

			// For now the transmitted audio is always mono -> channelCount = 1
			// As the events may casue the output PCM to change, the connection has to be direct in any case
			if (user) {
				emit audioSourceFetched(pfBuffer, nsamp, 1, true, user);
			} else {
				emit audioSourceFetched(pfBuffer, nsamp, 1, false, nullptr);
			}

			// If recording is enabled add the current audio source to the recording buffer
			if (recorder) {
				AudioOutputSpeech *aos = qobject_cast<AudioOutputSpeech *>(aop);

				if (aos) {
					for (unsigned int i = 0; i < nsamp; ++i) {
						recbuff[i] += pfBuffer[i] * volumeAdjustment;
					}

					if (!recorder->isInMixDownMode()) {
						recorder->addBuffer(aos->p, recbuff, nsamp);
						recbuff = boost::shared_array<float>(new float[nsamp]);
						memset(recbuff.get(), 0, sizeof(float) * nsamp);
					}

					// Don't add the local audio to the real output
					if (qobject_cast<RecordUser *>(aos->p)) {
						continue;
					}
				}
			}

			if (validListener && ((aop->fPos[0] != 0.0f) || (aop->fPos[1] != 0.0f) || (aop->fPos[2] != 0.0f))) {
				// If positional audio is enabled, calculate the respective audio effect here
				Position3D outputPos = { aop->fPos[0], aop->fPos[1], aop->fPos[2]  };
				Position3D ownPos = g.pluginManager->getPositionalData().getCameraPos();

				Vector3D connectionVec = outputPos - ownPos;
				float len = connectionVec.norm();

				if (len > 0.0f) {

					// don't use normalize-func in order to save the re-computation of the vector's length
					connectionVec.x /= len;
					connectionVec.y /= len;
					connectionVec.z /= len;
				}
				/*
								qWarning("Voice pos: %f %f %f", aop->fPos[0], aop->fPos[1], aop->fPos[2]);
								qWarning("Voice dir: %f %f %f", dir[0], dir[1], dir[2]);
				*/
				if (! aop->pfVolume) {
					aop->pfVolume = new float[nchan];
					for (unsigned int s=0;s<nchan;++s)
						aop->pfVolume[s] = -1.0;
				}
				for (unsigned int s=0;s<nchan;++s) {
					const float dot = bSpeakerPositional[s] ? connectionVec.x * speaker[s*3+0] + connectionVec.y * speaker[s*3+1]
						+ connectionVec.z * speaker[s*3+2] : 1.0f;
					const float str = svol[s] * calcGain(dot, len) * volumeAdjustment;
					float * RESTRICT o = output + s;
					const float old = (aop->pfVolume[s] >= 0.0f) ? aop->pfVolume[s] : str;
					const float inc = (str - old) / static_cast<float>(nsamp);
					aop->pfVolume[s] = str;
					/*
										qWarning("%d: Pos %f %f %f : Dot %f Len %f Str %f", s, speaker[s*3+0], speaker[s*3+1], speaker[s*3+2], dot, len, str);
					*/
					if ((old >= 0.00000001f) || (str >= 0.00000001f))
						for (unsigned int i=0;i<nsamp;++i)
							o[i*nchan] += pfBuffer[i] * (old + inc*static_cast<float>(i));
				}
			} else {
				// Mix the current audio source into the output by adding it to the elements of the output buffer after having applied
				// a volume adjustment
				for (unsigned int s=0;s<nchan;++s) {
					const float str = svol[s] * volumeAdjustment;
					float * RESTRICT o = output + s;
					for (unsigned int i=0;i<nsamp;++i)
						o[i*nchan] += pfBuffer[i] * str;
				}
			}
		}

		if (recorder && recorder->isInMixDownMode()) {
			recorder->addBuffer(NULL, recbuff, nsamp);
		}

		emit audioOutputAboutToPlay(output, nsamp, nchan);

		// Clip the output audio
		if (eSampleFormat == SampleFloat)
			for (unsigned int i=0;i<nsamp*iChannels;i++)
				output[i] = qBound(-1.0f, output[i], 1.0f);
		else
			// Also convert the intermediate float array into an array of shorts before writing it to the outbuff
			for (unsigned int i=0;i<nsamp*iChannels;i++)
				reinterpret_cast<short *>(outbuff)[i] = static_cast<short>(qBound(-32768.f, (output[i] * 32768.f), 32767.f));
	}

	qrwlOutputs.unlock();

	// Delete all AudioOutputUsers that no longer provide any new audio
	foreach(AudioOutputUser *aop, qlDel)
		removeBuffer(aop);
	
	// Return whether data has been written to the outbuff
	return (! qlMix.isEmpty());
}

bool AudioOutput::isAlive() const {
	return isRunning();
}

unsigned int AudioOutput::getMixerFreq() const {
	return iMixerFreq;
}

