#! /usr/bin/env bash
# SPDX-FileCopyrightText: none
# SPDX-License-Identifier: CC0-1.0
$XGETTEXT `find -name \*.cpp -o -name \*.qml -o -name \*.js` -o $podir/kontainer.pot
