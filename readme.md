Audiobook Player
Một trình phát sách nói (audiobook) đơn giản, mạnh mẽ được xây dựng bằng C++ và Qt 6 cho nền tảng Windows 64-bit.

Tác giả: Google Gemini

Giới thiệu
Audiobook Player được thiết kế để cung cấp một trải nghiệm nghe sách nói mượt mà và tập trung. Ứng dụng tự động quét thư viện của bạn, lấy thông tin sách và chương, đồng thời lưu lại tiến độ nghe của bạn một cách thông minh.

Tính năng chính
Quản lý Thư viện Thông minh: Tự động quét các thư mục con để nhận diện sách và các file âm thanh.

Trình phát Đầy đủ Chức năng: Bao gồm các điều khiển phát/dừng, chuyển chương, tua tới/lui, điều chỉnh tốc độ và âm lượng.

Lưu trữ Tiến độ: Tự động lưu lại vị trí bạn đang nghe dở cho từng chương và tổng tiến độ cho từng cuốn sách.

Giao diện Trực quan: Hỗ trợ hiển thị ảnh bìa, thông tin sách và chương rõ ràng.

Tính năng Tiện ích: Bao gồm hẹn giờ tắt, nhảy đến một thời điểm cụ thể trong chương.

Hỗ trợ Đa định dạng: Nhờ vào FFmpeg, ứng dụng có thể phát hầu hết các định dạng âm thanh phổ biến.

Hướng dẫn Biên dịch và Chạy dự án
Để biên dịch dự án, vui lòng tham khảo file YEU_CAU.md để đảm bảo bạn đã cài đặt đầy đủ môi trường cần thiết.

Sau khi đã đáp ứng tất cả các yêu cầu, bạn chỉ cần chạy file build_release.bat có sẵn trong thư mục gốc của dự án.

Kịch bản sẽ tự động thực hiện các công việc sau:

Tìm và cấu hình môi trường Visual Studio 2022.

Tạo thư mục build.

Cấu hình dự án bằng CMake.

Biên dịch dự án ở chế độ Release.

Sao chép các file DLL cần thiết của Qt và FFmpeg vào thư mục build/Release.

Sau khi kịch bản chạy thành công, bạn có thể tìm thấy file AudiobookPlayer.exe và tất cả các file cần thiết để chạy ứng dụng trong thư mục build\Release.

Dự án được tạo và cải tiến bởi Google Gemini, 2025.