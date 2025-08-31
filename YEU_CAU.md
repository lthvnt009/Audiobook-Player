Yêu cầu Hệ thống & Cài đặt Môi trường
Để có thể biên dịch và phát triển dự án Audiobook Player, máy tính của bạn cần đáp ứng các yêu cầu sau.

1. Yêu cầu Hệ thống
Hệ điều hành: Windows 10 hoặc Windows 11 (phiên bản 64-bit).

RAM: Tối thiểu 8 GB.

Dung lượng ổ cứng: Khoảng 15-20 GB trống để cài đặt các công cụ cần thiết.

2. Các Công cụ Bắt buộc
Visual Studio 2022:

Tải và cài đặt phiên bản Community (miễn phí) từ trang chủ của Microsoft.

Trong quá trình cài đặt, hãy chắc chắn chọn Workload "Desktop development with C++".

CMake:

Dự án yêu cầu CMake phiên bản 3.20 trở lên.

CMake thường được tích hợp sẵn khi bạn cài đặt Visual Studio với workload ở trên. Bạn không cần cài đặt riêng lẻ trừ khi có sự cố.

Thư viện Qt 6:

Dự án được phát triển và kiểm thử với Qt 6.9.1.

QUAN TRỌNG: Bạn phải cài đặt Qt vào đúng đường dẫn sau: C:\Qt\6.9.1.

Khi cài đặt, hãy chọn đúng phiên bản compiler là MSVC 2022 64-bit.

3. Các Thư viện Phụ thuộc
FFmpeg:

Dự án yêu cầu thư viện FFmpeg để giải mã các file âm thanh.

Tải phiên bản full_build-shared mới nhất cho Windows từ trang chủ FFmpeg hoặc các nguồn build uy tín (ví dụ: gyan.dev).

Giải nén toàn bộ thư mục đã tải về và đổi tên nó thành ffmpeg.

Đặt thư mục ffmpeg này vào thư mục gốc của dự án, ngang hàng với file CMakeLists.txt.

4. Cấu trúc Thư mục Dự án (Bắt buộc)
Sau khi chuẩn bị xong, cấu trúc thư mục gốc của dự án của bạn phải trông như sau:

Audiobook-Player/
├── src/
│   ├── MainWindow.cpp
│   ├── MainWindow.h
│   └── ... (tất cả các file .cpp và .h khác)
├── ffmpeg/
│   ├── bin/
│   ├── include/
│   └── lib/
├── build_release.bat
├── CMakeLists.txt
├── README.md
└── ... (các file khác)

Khi tất cả các yêu cầu trên đã được đáp ứng, bạn đã sẵn sàng để biên dịch dự án bằng cách chạy file build_release.bat.