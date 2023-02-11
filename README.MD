Hướng dẫn chạy file.
Bước 1: chạy server: ./server
Bước 2: Chạy client với argv như sau:
./client + 127.0.0.1 + port của server (8089) + port muốn share file (để 9090)
client 2 chạy tương tự với port share = 9091
ex: ./client 127.0.0.1 8089 9090
./client 127.0.0.1 8089 9090

Hướng dẫn sử dụng command:
inforallfiles : in ra tất cả các file đang được share
myfiles: in ra file mà máy này đang share
deletefile + id, id hiển thị đầu tiên khi chạy 2 lệnh trên, 1,2,3,4..... dùng để xóa file bản thân đang share
uploadfile + tên file (hoặc đường dẫn), để tải file muốn chia sẻ lên server
downloadfile + id, tải file đang được chia sẻ