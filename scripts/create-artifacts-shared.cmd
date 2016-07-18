mkdir C:\projects\libui\tmp
copy C:\projects\libui\tmp-64\libui.* C:\projects\libui\tmp
copy C:\projects\libui\tmp-64\*.h C:\projects\libui\tmp

7z a -ttar libui-shared-windows-x64-%APPVEYOR_REPO_TAG_NAME%.tar C:\projects\libui\tmp\*.*
7z a -tgzip libui-shared-windows-x64-%APPVEYOR_REPO_TAG_NAME%.tar.gz libui-shared-windows-x64-%APPVEYOR_REPO_TAG_NAME%.tar

del C:\projects\libui\tmp\*.* /q
copy C:\projects\libui\tmp-64\*.* C:\projects\libui\tmp

7z a -ttar examples-shared-windows-x64-%APPVEYOR_REPO_TAG_NAME%.tar C:\projects\libui\tmp\*.*
7z a -tgzip examples-shared-windows-x64-%APPVEYOR_REPO_TAG_NAME%.tar.gz examples-shared-windows-x64-%APPVEYOR_REPO_TAG_NAME%.tar

del C:\projects\libui\tmp\*.* /q

copy C:\projects\libui\tmp-32\libui.* C:\projects\libui\tmp
copy C:\projects\libui\tmp-32\*.h C:\projects\libui\tmp

7z a -ttar libui-shared-windows-ia32-%APPVEYOR_REPO_TAG_NAME%.tar C:\projects\libui\tmp\*.*
7z a -tgzip libui-shared-windows-ia32-%APPVEYOR_REPO_TAG_NAME%.tar.gz libui-shared-windows-ia32-%APPVEYOR_REPO_TAG_NAME%.tar

del C:\projects\libui\tmp\*.* /q
copy C:\projects\libui\tmp-32\*.* C:\projects\libui\tmp

7z a -ttar examples-shared-windows-ia32-%APPVEYOR_REPO_TAG_NAME%.tar C:\projects\libui\tmp\*.*
7z a -tgzip examples-shared-windows-ia32-%APPVEYOR_REPO_TAG_NAME%.tar.gz examples-shared-windows-x32-%APPVEYOR_REPO_TAG_NAME%.tar
