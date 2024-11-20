@ECHO OFF

SET CYGWIN_BIN=D:\WXWork\myfiles\LCS6260-TR6260\Tools\Toolchain\windows\cygwin\bin
SET COMPILER_BIN=D:\WXWork\myfiles\LCS6260-TR6260\Tools\Toolchain\windows\nds32le-elf-mculib-v3\bin
SET PATH=%COMPILER_BIN%;%CYGWIN_BIN%;%PATH%
SET HOME=%~dp0

bash --login -i
