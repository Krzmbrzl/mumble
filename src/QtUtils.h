// Copyright 2021 The Mumble Developers. All rights reserved.
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file at the root of the
// Mumble source tree or at <https://www.mumble.info/LICENSE>.

#ifndef MUMBLE_QTUTILS_H_
#define MUMBLE_QTUTILS_H_

#include <QString>

#include <memory>

class QObject;
class QStringList;

namespace Mumble {
namespace QtUtils {

	QString decode_utf8_qssl_string(const QString &input);

	/**
	 * Applies decode_utf8_qssl_string on the first element in the
	 * given list. If the list is empty an empty String is returned.
	 */
	QString decode_first_utf8_qssl_string(const QStringList &list);

	struct QObjectDeleter {
		void operator()(QObject *obj);
	};

	template< typename T > using qobject_unique_ptr = std::unique_ptr< T, QObjectDeleter >;

	template< typename T, class... Args > qobject_unique_ptr< T > make_unique_qobject(Args &&... args) {
		return qobject_unique_ptr< T >(new T(std::forward< Args >(args)...));
	}

}; // namespace QtUtils
}; // namespace Mumble

#endif // MUMBLE_QTUTILS_H_
