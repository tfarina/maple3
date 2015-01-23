#!/bin/bash

# https://bitbucket.org/xerial/sqlite-jdbc#markdown-header-usage

mkdir -p build/classes
javac -cp lib/* -sourcepath com/zetcode -d build/classes com/zetcode/Sqlite3Test.java
java -cp lib/*:build/classes com.zetcode.Sqlite3Test
