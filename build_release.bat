@echo off
setlocal

:: =================================================================================
:: KỊCH BẢN BUILD PHIÊN BẢN RELEASE CHO DỰ ÁN AUDIOBOOK PLAYER
:: Tác giả: Kiến Trúc Sư Lập Trình AI
:: Phiên bản: 2.7 (Loại bỏ UPX để đảm bảo ổn định)
:: Mục đích: Tự động hóa quá trình biên dịch và triển khai ứng dụng.
:: =================================================================================

echo [INFO] Bat dau qua trinh build phien ban Release...
echo.

:: --- BUOC -1: TIM VA CAU HINH MOI TRUONG VISUAL STUDIO ---
echo [INFO] Buoc -1: Tim kiem va cau hinh moi truong Visual Studio...

:: Tim duong dan cai dat Visual Studio 2022 bang vswhere.exe
for /f "usebackq tokens=*" %%i in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -property installationPath`) do (
  set "VS_INSTALL_DIR=%%i"
)

if not defined VS_INSTALL_DIR (
    echo [ERROR] Khong tim thay Visual Studio 2022. Vui long cai dat Visual Studio 2022 Community voi workload 'Desktop development with C++'.
    goto :error
)

:: Duong dan den file vcvarsall.bat, dung de thiet lap moi truong build
set "VCVARSALL_BAT=%VS_INSTALL_DIR%\VC\Auxiliary\Build\vcvarsall.bat"

if not exist "%VCVARSALL_BAT%" (
    echo [ERROR] Khong tim thay vcvarsall.bat tai "%VCVARSALL_BAT%".
    echo [ERROR] Vui long dam bao workload 'Desktop development with C++' da duoc cai dat trong Visual Studio.
    goto :error
)

echo [INFO] Da tim thay Visual Studio tai: %VS_INSTALL_DIR%
echo [INFO] Dang cau hinh moi truong build cho x64...
call "%VCVARSALL_BAT%" x64
echo [INFO] Moi truong build da san sang.

:: --- BUOC 0: CAU HINH BIEN MOI TRUONG ---
echo.
echo [INFO] Buoc 0: Kiem tra va cau hinh du an...

:: **QUAN TRONG:** Duong dan den thu muc cai dat Qt.
set "QT_INSTALL_DIR=C:\Qt\6.9.1\msvc2022_64"

:: Kiem tra xem duong dan Qt co hop le khong
if not exist "%QT_INSTALL_DIR%\bin\windeployqt.exe" (
    echo [ERROR] Khong tim thay windeployqt.exe tai "%QT_INSTALL_DIR%\bin\".
    echo [ERROR] Vui long kiem tra lai duong dan 'QT_INSTALL_DIR' trong file nay.
    goto :error
)
echo [INFO] Da tim thay Qt tai: %QT_INSTALL_DIR%

:: Thiet lap cac duong dan can thiet
set "PROJECT_DIR=%~dp0"
set "BUILD_DIR=%PROJECT_DIR%build"
set "RELEASE_DIR=%BUILD_DIR%\Release"
set "PROJECT_NAME=AudiobookPlayer"

:: Chuyen thu muc lam viec ve thu muc goc cua du an
cd /d "%PROJECT_DIR%"


:: --- BUOC 1: DON DEP ---
echo.
echo [INFO] Buoc 1: Don dep thu muc build cu...
echo [INFO] Luu y: Neu buoc nay that bai, hay dam bao ban da dong VS Code va ung dung.
if exist "%BUILD_DIR%" (
    rmdir /S /Q "%BUILD_DIR%"
)
mkdir "%BUILD_DIR%"
echo [INFO] Da don dep xong.


:: --- BUOC 2: CAU HINH DU AN VOI CMAKE ---
echo.
echo [INFO] Buoc 2: Cau hinh du an voi CMake...

cmake -S . -B "%BUILD_DIR%" -G "Visual Studio 17 2022" -A x64 -DCMAKE_PREFIX_PATH="%QT_INSTALL_DIR%"
if %errorlevel% neq 0 (
    echo [ERROR] Qua trinh cau hinh CMake that bai.
    goto :error
)
echo [INFO] Da cau hinh CMake thanh cong.


:: --- BUOC 3: BIEN DICH DU AN (BUILD RELEASE) ---
echo.
echo [INFO] Buoc 3: Bien dich du an (Che do Release)...

cmake --build "%BUILD_DIR%" --config Release
if %errorlevel% neq 0 (
    echo [ERROR] Qua trinh bien dich that bai.
    goto :error
)
echo [INFO] Da bien dich du an thanh cong. File .exe duoc dat tai: %RELEASE_DIR%


:: --- BUOC 4: TRIEN KHAI (DEPLOY) - SAO CHEP DLLs ---
echo.
echo [INFO] Buoc 4: Sao chep cac file thu vien can thiet...

echo [INFO] Dang chay windeployqt de sao chep DLLs cua Qt...
"%QT_INSTALL_DIR%\bin\windeployqt.exe" --release --no-translations --no-opengl-sw "%RELEASE_DIR%\%PROJECT_NAME%.exe"
if %errorlevel% neq 0 (
    echo [ERROR] Qua trinh chay windeployqt that bai.
    goto :error
)

echo [INFO] Dang sao chep cac file DLL can thiet cua FFmpeg...
copy /Y "ffmpeg\bin\avcodec-*.dll" "%RELEASE_DIR%\"
copy /Y "ffmpeg\bin\avformat-*.dll" "%RELEASE_DIR%\"
copy /Y "ffmpeg\bin\avutil-*.dll" "%RELEASE_DIR%\"
copy /Y "ffmpeg\bin\swresample-*.dll" "%RELEASE_DIR%\"
copy /Y "ffmpeg\bin\avfilter-*.dll" "%RELEASE_DIR%\"
copy /Y "ffmpeg\bin\postproc-*.dll" "%RELEASE_DIR%\"


:: ==================== BẮT ĐẦU THAY ĐỔI ====================
:: Loai bo hoan toan buoc nen file UPX de dam bao tinh on dinh toi da
:: --- BUOC 5 (TUY CHON): NEN FILE VOI UPX ---
:: ===================== KẾT THÚC THAY ĐỔI =====================


:success
echo.
echo [SUCCESS] HOAN TAT!
echo Ban co the tim thay phien ban Release hoan chinh tai:
echo %RELEASE_DIR%
echo.
goto :end

:error
echo.
echo [FAILED] Qua trinh build da that bai. Vui long kiem tra lai cac thong bao loi o tren.
echo.

:end
pause
endlocal

