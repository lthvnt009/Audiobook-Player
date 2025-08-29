@echo off
setlocal

:: =================================================================================
:: KỊCH BẢN BUILD PHIÊN BẢN RELEASE CHO DỰ ÁN AUDIOBOOK PLAYER
:: =================================================================================
::
:: Mục đích:
:: 1. Dọn dẹp thư mục build cũ.
:: 2. Cấu hình dự án bằng CMake.
:: 3. Biên dịch dự án ở chế độ Release (tối ưu hóa).
:: 4. Tự động sao chép các file DLL cần thiết để tạo gói cài đặt hoàn chỉnh.
::

:: --- BƯỚC 0: CẤU HÌNH BIẾN MÔI TRƯỜNG ---
echo [INFO] Buoc 0: Cau hinh bien moi truong...

:: **QUAN TRỌNG:** Vui lòng chỉnh sửa đường dẫn này cho đúng với máy của bạn!
set "QT_INSTALL_DIR=C:\Qt\6.9.1\msvc2022_64"

:: Kiểm tra xem đường dẫn Qt có hợp lệ không
if not exist "%QT_INSTALL_DIR%\bin\windeployqt.exe" (
    echo [ERROR] Khong tim thay windeployqt.exe tai "%QT_INSTALL_DIR%\bin\".
    echo [ERROR] Vui long kiem tra lai duong dan 'QT_INSTALL_DIR' trong file build.bat.
    goto :eof
)
echo [INFO] Da tim thay Qt tai: %QT_INSTALL_DIR%

:: Thêm thư mục bin của Qt vào PATH
set "PATH=%QT_INSTALL_DIR%\bin;%PATH%"
set "PROJECT_DIR=%~dp0"
set "BUILD_DIR=%PROJECT_DIR%build"
set "RELEASE_DIR=%BUILD_DIR%\Release"

:: Chuyển thư mục làm việc về thư mục gốc của dự án
cd /d "%PROJECT_DIR%"

:: --- BƯỚC 1: DỌN DẸP ---
echo.
echo [INFO] Buoc 1: Don dep thu muc build cu...
if exist "build" (
    rmdir /S /Q "build"
)
mkdir "build"
echo [INFO] Da don dep xong.

:: --- BƯỚC 2: CẤU HÌNH DỰ ÁN VỚI CMAKE ---
echo.
echo [INFO] Buoc 2: Cau hinh du an voi CMake...

cmake -S . -B "build" -G "Visual Studio 17 2022" -A x64
if %errorlevel% neq 0 (
    echo [ERROR] Qua trinh cau hinh CMake that bai.
    goto :eof
)
echo [INFO] Da cau hinh CMake thanh cong.

:: --- BƯỚC 3: BIÊN DỊCH DỰ ÁN (BUILD RELEASE) ---
echo.
echo [INFO] Buoc 3: Bien dich du an (Che do Release)...

cmake --build "build" --config Release
if %errorlevel% neq 0 (
    echo [ERROR] Qua trinh bien dich that bai.
    goto :eof
)
echo [INFO] Da bien dich du an thanh cong. File .exe duoc dat tai: %RELEASE_DIR%

:: --- BƯỚC 4: TRIỂN KHAI (DEPLOY) - SAO CHÉP DLLs ---
echo.
echo [INFO] Buoc 4: Sao chep cac file DLL can thiet...

echo [INFO] Dang sao chep cac file DLL cua Qt (Release)...
windeployqt.exe --release --no-translations --no-opengl-sw "%RELEASE_DIR%\AudiobookPlayer.exe"
if %errorlevel% neq 0 (
    echo [ERROR] Qua trinh chay windeployqt that bai.
    goto :eof
)

echo [INFO] Dang sao chep cac file DLL cua FFmpeg...
xcopy /Y /Q "ffmpeg\bin\*.dll" "%RELEASE_DIR%\"
if %errorlevel% neq 0 (
    echo [ERROR] Khong the sao chep file DLL cua FFmpeg.
    goto :eof
)

echo.
echo [SUCCESS] HOAN TAT!
echo Ban co the tim thay phien ban Release tai: %RELEASE_DIR%
echo.

pause
endlocal
