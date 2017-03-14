@if (@a==@b) @end /*

:: unzip.bat
:: usage: unzip.bat zipfile.zip

:: begin batch portion

@echo off
setlocal

if "%~1"=="" (
    echo Usage: %~nx0 filename.zip
    goto :EOF
)

cscript /nologo /e:jscript "%~f0" "%~f1"

goto :EOF

:: end batch portion
:: begin JScript portion */

function unzip(zipfile, unzipdir) {
    var fso = new ActiveXObject('Scripting.FileSystemObject'),
        shell = new ActiveXObject('Shell.Application'),
        dst, zip;

    if (!unzipdir) unzipdir = '.';

    if (!fso.FolderExists(unzipdir)) fso.CreateFolder(unzipdir);

    dst = shell.NameSpace(fso.getFolder(unzipdir).Path);
    zip = shell.NameSpace(fso.getFile(zipfile).Path);

    for (var i=0; i<zip.Items().Count; i++) {
        try {
            if (fso.FileExists(zipfile)) {
                WSH.Stdout.Write('Unzipping ' + zip.Items().Item(i) + '... ');
                dst.CopyHere(zip.Items().Item(i), 4 + 16);
                WSH.Echo('Done.');
            }
        }
        catch(e) {
            WSH.Echo('Failed: ' + e);
        }
    }
}

var zipfile = WSH.Arguments(0),
dest = zipfile.match(/(.*)[\/\\]/)[1]||'';

unzip(zipfile, dest);

