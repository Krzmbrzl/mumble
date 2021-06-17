// Copyright 2021 The Mumble Developers. All rights reserved.
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file at the root of the
// Mumble source tree or at <https://www.mumble.info/LICENSE>.

#ifndef MUMBLE_MUMBLE_AUDIENCEDIALOG_H_
#define MUMBLE_MUMBLE_AUDIENCEDIALOG_H_

#include <QDialog>
#include <QVector>

#include <memory>

#include "ui_AudienceDialog.h"

class AudienceDialog : public QDialog, Ui::AudienceDialog {
public:
	AudienceDialog(const QVector<unsigned int> &sessions = {}, QWidget *parent = nullptr);
};

#endif // MUMBLE_MUMBLE_AUDIENCEDIALOG_H_
