
@ECHO OFF
set SPHINXBUILD=sphinx-build
set SOURCEDIR=.
set BUILDDIR=_build
if "%1" == "html" (
    %SPHINXBUILD% -b html %SOURCEDIR% %BUILDDIR%/html
) else (
    %SPHINXBUILD% -M %1 %SOURCEDIR% %BUILDDIR%
)
