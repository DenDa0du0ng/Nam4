#define _GNU_SOURCE
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/user.h>

// định nghĩa một macro PAGE_SIZE có giá trị là 4096. Macro PAGE_SIZE có giá trị là 4096, và được sử dụng để đại diện cho kích thước của trang (page) trên hệ thống.
#ifndef PAGE_SIZE
#define PAGE_SIZE 4096
#endif

/**
 * Create a pipe where all "bufs" on the pipe_inode_info ring have the
 * PIPE_BUF_FLAG_CAN_MERGE flag set.
 */
//tuyền vào một mảng hai phần tử đại diện cho 2 đầu của pipe p[0] là đầu đọc p[1] là đầu ghi
// hàm này sẽ tạo ra một pipe và điền dữ liệu vào pipe cho đến khi pipe được đầy hoàn toàn. Để làm được điều này, ta cần biết kích thước hiện tại của pipe, được lấy từ pipe_size, và ta sử dụng một buffer (buffer) để chứa dữ liệu sẽ được ghi vào pipe.
// Khi dữ liệu được ghi vào pipe, kernel sẽ tạo ra các buffer để lưu trữ dữ liệu đó, các buffer này có kích thước bằng với kích thước của trang (page) trên hệ thống. Việc đọc và ghi dữ liệu sử dụng mảng buffer này sẽ giúp ta ghi hoặc đọc các buffer của pipe một cách dễ dàng.

static void prepare_pipe(int p[2])
{
    //tạo một pipe nếu không thành công thì thoát luôn thông qua hàm abort
    // pipe() tạo ra một pipe và trả về hai file descriptor, một cho đầu đọc (read end) và một cho đầu ghi (write end) của pipe. Đầu đầu tiên của pipe được đặt tại p[0] và đầu thứ hai được đặt tại p[1]. Khi một tiến trình ghi dữ liệu vào đầu thứ hai của pipe (p[1]), tiến trình khác có thể đọc dữ liệu đó từ đầu đầu tiên của pipe (p[0]).
	if (pipe(p)) abort();

// ta cần lấy kích thước của pipe bằng cách gọi hàm fcntl() 
// với tham số là file descriptor của đầu ghi (p[1]) và F_GETPIPE_SZ.
// Mảng p là một mảng có hai phần tử, mỗi phần tử đại diện cho một đầu của pipe. Đầu đầu tiên của pipe được đặt tại p[0] và đầu thứ hai được đặt tại p[1]. Khi một tiến trình ghi dữ liệu vào đầu thứ hai của pipe (p[1]), tiến trình khác có thể đọc dữ liệu đó từ đầu đầu tiên của pipe (p[0]).
// File descriptor là một con số duy nhất được gán cho một tập tin, ổ đĩa hoặc một socket, được sử dụng để truy cập vào tài nguyên đó. Trong trường hợp này, hai file descriptor được truyền vào hàm pipe() để tạo ra pipe. File descriptor thứ nhất (p[0]) được sử dụng để đọc dữ liệu từ pipe, và file descriptor thứ hai (p[1]) được sử dụng để ghi dữ liệu vào pipe.
// Khi ta gọi fcntl() với tham số là file descriptor của một pipe và tham số F_GETPIPE_SZ, hàm này sẽ trả về kích thước của pipe đó. Các phần tử của mảng p được sử dụng để truyền vào hàm pipe() và fcntl() để tạo ra pipe và lấy kích thước của pipe.
// do tính chất của pipe trong Linux. Một pipe có thể có nhiều đầu đọc (read end) nhưng chỉ có một đầu ghi (write end), do đó khi muốn lấy kích thước của pipe, ta sử dụng đầu ghi của pipe (file descriptor p[1]) là đối tượng tham số cho hàm fcntl() với F_GETPIPE_SZ.
// ta cần lấy kích thước của pipe là vì ta cần điền dữ liệu vào pipe cho đến khi pipe được đầy hoàn toàn. Để làm được điều này, ta cần biết kích thước hiện tại của pipe, được lấy từ pipe_size, và ta sử dụng một buffer (buffer) để chứa dữ liệu sẽ được ghi vào pipe.
// Khi một buffer của pipe được tạo ra, nó sẽ được đánh dấu với cờ PIPE_BUF_FLAG_CAN_MERGE. Vì vậy, khi pipe được điền đầy, ta sẽ có một số lượng buffer của pipe được tạo ra, mỗi buffer có kích thước bằng với kích thước của trang (page) trên hệ thống và được đánh dấu với cờ PIPE_BUF_FLAG_CAN_MERGE.

   const unsigned pipe_size = fcntl(p[1], F_GETPIPE_SZ);

// Dòng lệnh này khai báo một mảng ký tự buffer với kích thước 4096. Mảng này được sử dụng để ghi hoặc đọc dữ liệu từ pipe.
// Trong hàm prepare_pipe(), buffer được sử dụng để đọc và ghi dữ liệu vào pipe. Để chuẩn bị cho pipe, hàm này sẽ ghi đủ dữ liệu vào pipe để nó tràn đầy, đồng thời đọc hết dữ liệu khỏi pipe để giải phóng tất cả các buffer của pipe.
// Khi dữ liệu được ghi vào pipe, kernel sẽ tạo ra các buffer để lưu trữ dữ liệu đó, các buffer này có kích thước bằng với kích thước của trang (page) trên hệ thống. Việc đọc và ghi dữ liệu sử dụng mảng buffer này sẽ giúp ta ghi hoặc đọc các buffer của pipe một cách dễ dàng.
// Kích thước 4096 được chọn là vì đó là kích thước mặc định của trang (page) trên hầu hết các hệ thống Linux hiện đại. Mỗi buffer của pipe được tạo ra với kích thước bằng với kích thước của trang, vì vậy sử dụng mảng buffer với kích thước 4096 cho phép ta ghi hoặc đọc dữ liệu một cách hiệu quả từ các buffer của pipe
    static char buffer[4096];

	/* fill the pipe completely; each pipe_buffer will now have
	   the PIPE_BUF_FLAG_CAN_MERGE flag */

// Vòng lặp này có chức năng điền dữ liệu vào pipe. Cụ thể, vòng lặp này sẽ ghi dữ liệu vào đầu ghi của pipe (p[1]) cho đến khi pipe được đầy hoàn toàn. Để làm được điều này, ta cần biết kích thước hiện tại của pipe, được lấy từ pipe_size, và ta sử dụng một buffer (buffer) để chứa dữ liệu sẽ được ghi vào pipe.
// Trong vòng lặp này, biến r biểu thị số byte còn lại trong pipe mà chưa được điền dữ liệu vào, ban đầu có giá trị bằng pipe_size. Sau đó, biến n được tính toán là kích thước của buffer dữ liệu mà ta sẽ ghi vào pipe trong lần lặp này. Nếu kích thước của pipe còn lại (r) lớn hơn kích thước của buffer (sizeof(buffer)), ta sẽ ghi vào pipe một lượng dữ liệu có kích thước bằng với kích thước của buffer (n = sizeof(buffer)). Nếu kích thước của pipe còn lại nhỏ hơn kích thước của buffer, ta sẽ chỉ ghi vào pipe số byte còn lại (n = r).
// Sau mỗi lần ghi dữ liệu vào pipe, ta cập nhật giá trị của biến r bằng cách trừ đi kích thước của buffer dữ liệu vừa được ghi vào pipe (r -= n). Vòng lặp sẽ tiếp tục cho đến khi pipe được điền đầy hoàn toàn.
// Khi pipe được điền đầy, ta sẽ có một số lượng buffer của pipe được tạo ra, mỗi buffer có kích thước bằng với kích thước của trang (page) trên hệ thống. Vì vậy, khi pipe được điền đầy, ta sẽ có một số lượng buffer của pipe có kích thước bằng với kích thước của trang (page) trên hệ thống.
// Khi một buffer của pipe được tạo ra, nó sẽ được đánh dấu với cờ PIPE_BUF_FLAG_CAN_MERGE. Vì vậy, khi pipe được điền đầy, ta sẽ có một số lượng buffer của pipe được tạo ra, mỗi buffer có kích thước bằng với kích thước của trang (page) trên hệ thống và được đánh dấu với cờ PIPE_BUF_FLAG_CAN_MERGE.
// Vì vậy, khi pipe được điền đầy, ta sẽ có một số lượng buffer của pipe được tạo ra, mỗi buffer có kích thước bằng với kích thước của trang (page) trên hệ thống và được đánh dấu với cờ PIPE_BUF_FLAG_CAN_MERGE. Vì vậy, khi pipe được điền đầy, ta sẽ có một số lượng buffer của pipe được tạo ra, mỗi buffer có kích thước bằng với kích thước của trang (page) trên hệ thống và được đánh dấu với cờ PIPE_BUF_FLAG_CAN_MERGE.

    for (unsigned r = pipe_size; r > 0;) {
		unsigned n = r > sizeof(buffer) ? sizeof(buffer) : r;
		write(p[1], buffer, n);
		r -= n;
	}

	/* drain the pipe, freeing all pipe_buffer instances (but
	   leaving the flags initialized) */
// vòng for này để đọc hết dữ liệu trong pipe, khi đọc hết dữ liệu trong pipe thì các buffer của pipe sẽ được giải phóng, nhưng các cờ PIPE_BUF_FLAG_CAN_MERGE vẫn được giữ nguyên.
// Vòng lặp này có chức năng đọc dữ liệu từ pipe. Cụ thể, vòng lặp này sẽ đọc dữ liệu từ đầu đọc của pipe (p[0]) cho đến khi pipe trống hoàn toàn. Để làm được điều này, ta cần biết kích thước hiện tại của pipe, được lấy từ pipe_size, và ta sử dụng một buffer (buffer) để chứa dữ liệu sẽ được đọc từ pipe.
// Trong vòng lặp này, biến r biểu thị số byte còn lại trong pipe mà chưa được đọc, ban đầu có giá trị bằng pipe_size. Sau đó, biến n được tính toán là kích thước của buffer dữ liệu mà ta sẽ đọc từ pipe trong lần lặp này. Nếu kích thước của pipe còn lại (r) lớn hơn kích thước của buffer (sizeof(buffer)), ta sẽ đọc từ pipe một lượng dữ liệu có kích thước bằng với kích thước của buffer (n = sizeof(buffer)). Nếu kích thước của pipe còn lại nhỏ hơn kích thước của buffer, ta sẽ chỉ đọc từ pipe số byte còn lại (n = r).
// Sau mỗi lần đọc dữ liệu từ pipe, ta cập nhật giá trị của biến r bằng cách trừ đi kích thước của buffer dữ liệu vừa được đọc từ pipe (r -= n). Vòng lặp sẽ tiếp tục cho đến khi pipe trống hoàn toàn.
// Khi pipe trống hoàn toàn, ta sẽ có một số lượng buffer của pipe được giải phóng, nhưng các cờ PIPE_BUF_FLAG_CAN_MERGE vẫn được giữ nguyên. Vì vậy, khi pipe trống hoàn toàn, ta sẽ có một số lượng buffer của pipe được giải phóng, nhưng các cờ PIPE_BUF_FLAG_CAN_MERGE vẫn được giữ nguyên.

	for (unsigned r = pipe_size; r > 0;) {
		unsigned n = r > sizeof(buffer) ? sizeof(buffer) : r;
		read(p[0], buffer, n);
		r -= n;
	}

	/* the pipe is now empty, and if somebody adds a new
	   pipe_buffer without initializing its "flags", the buffer
	   will be mergeable  */
}
// hai tham số argc và **argv là các tham số dòng lệnh được truyền vào khi chạy chương trình. Tham số argc là số lượng tham số dòng lệnh được truyền vào, và tham số argv là một mảng các chuỗi ký tự, mỗi phần tử của mảng này là một tham số dòng lệnh.
// tham số dòng lệnh là các tham số được truyền vào khi chạy chương trình. Ví dụ, khi chạy chương trình với lệnh ./hello world, thì tham số dòng lệnh là world. Trong trường hợp này, argc sẽ có giá trị là 2, và argv sẽ là một mảng có hai phần tử, phần tử đầu tiên là ./hello và phần tử thứ hai là world.
// ví dụ khi chạy chương trình ./hello world 1 2 3 4 5, thì tham số argc sẽ có giá trị là 7, và tham số argv sẽ là một mảng có 7 phần tử, phần tử đầu tiên là ./hello, phần tử thứ hai là world, phần tử thứ ba là 1, phần tử thứ tư là 2, phần tử thứ năm là 3, phần tử thứ sáu là 4, và phần tử thứ bảy là 5.

int main(int argc, char **argv)
{
    //đây là một if đơn giản để kiểm tra xem số lượng tham số dòng lệnh có đúng không. Nếu không đúng thì in ra một thông báo lỗi và thoát chương trình.
	if (argc != 4) {
		fprintf(stderr, "Usage: %s TARGETFILE OFFSET DATA\n", argv[0]);
		return EXIT_FAILURE;
	}

	/* dumb command-line argument parser */
    
    // tách các tham số dòng lệnh ra thành các biến riêng biệt để sử dụng. Tham số dòng lệnh đầu tiên là đường dẫn tới file cần ghi dữ liệu, tham số thứ hai là vị trí bắt đầu ghi dữ liệu, và tham số thứ ba là dữ liệu cần ghi.
    // const char *const path = argv[1] có nghĩa là khai báo một biến path có kiểu const char *const, và gán giá trị cho biến này là argv[1]. Kiểu const char *const có nghĩa là một con trỏ tới một chuỗi ký tự không thể thay đổi, và con trỏ này không thể thay đổi địa chỉ mà nó trỏ tới.
	const char *const path = argv[1];
    // loff_t offset = strtoul(argv[2], NULL, 0) có nghĩa là khai báo một biến offset có kiểu loff_t, và gán giá trị cho biến này là argv[2]. Kiểu loff_t là một kiểu số nguyên có kích thước 64 bit, và có thể được định nghĩa là long long int.
    // biến offset là vị trí bắt đầu ghi dữ liệu, và được chuyển từ kiểu chuỗi ký tự sang kiểu số nguyên bằng cách sử dụng hàm strtoul(). Hàm strtoul() có tham số đầu tiên là một chuỗi ký tự, tham số thứ hai là một con trỏ tới một con trỏ, và tham số thứ ba là một số nguyên. Hàm strtoul() sẽ chuyển chuỗi ký tự được truyền vào thành một số nguyên, và gán giá trị của con trỏ tham số thứ hai bằng con trỏ tới ký tự đầu tiên không phải là số trong chuỗi ký tự được truyền vào. Nếu tham số thứ ba là 0, hàm strtoul() sẽ tự động xác định kiểu của số nguyên được chuyển từ chuỗi ký tự.
	loff_t offset = strtoul(argv[2], NULL, 0);


    // const char *const data = argv[3] có nghĩa là khai báo một biến data có kiểu const char *const, và gán giá trị cho biến này là argv[3]. Kiểu const char *const có nghĩa là một con trỏ tới một chuỗi ký tự không thể thay đổi, và con trỏ này không thể thay đổi địa chỉ mà nó trỏ tới.
	const char *const data = argv[3];
    // const size_t data_size = strlen(data) có nghĩa là khai báo một biến data_size có kiểu const size_t, và gán giá trị cho biến này là strlen(data). Kiểu const size_t có nghĩa là một số nguyên không âm không thể thay đổi.
	const size_t data_size = strlen(data);

    // đây là một if đơn giản để kiểm tra xem vị trí bắt đầu ghi dữ liệu có đúng không. Nếu không đúng thì in ra một thông báo lỗi và thoát chương trình.
	if (offset % PAGE_SIZE == 0) {
		fprintf(stderr, "Sorry, cannot start writing at a page boundary\n");
		return EXIT_FAILURE;
	}

   // khởi tạo biến next_page có kiểu loff_t, và gán giá trị cho biến này là (offset | (PAGE_SIZE - 1)) + 1. Biến next_page là vị trí bắt đầu của trang tiếp theo, và được tính toán bằng cách lấy offset OR với PAGE_SIZE - 1, và cộng thêm 1.
	const loff_t next_page = (offset | (PAGE_SIZE - 1)) + 1;
   
   // khởi tạo biến end_offset có kiểu loff_t, và gán giá trị cho biến này là offset + (loff_t)data_size. Biến end_offset là vị trí kết thúc của dữ liệu cần ghi, và được tính toán bằng cách lấy offset cộng với kích thước của dữ liệu cần ghi.
	const loff_t end_offset = offset + (loff_t)data_size;
	
  // kiểm tra và so sánh end_offset với next_page, nếu end_offset lớn hơn next_page thì in ra một thông báo lỗi và thoát chương trình.
	if (end_offset > next_page) {
		fprintf(stderr, "Sorry, cannot write across a page boundary\n");
		return EXIT_FAILURE;
	}

	/* open the input file and validate the specified offset */
	// khởi tạo biến fd là giá trị của hàm open(path, O_RDONLY). Biến fd là file descriptor của file cần ghi dữ liệu, và được khởi tạo bằng cách gọi hàm open() với tham số đầu tiên là đường dẫn tới file cần ghi dữ liệu, và tham số thứ hai là O_RDONLY.
	// nhắc lại chút file descriptor là một con số duy nhất được gán cho một tập tin, ổ đĩa hoặc một socket, được sử dụng để truy cập vào tài nguyên đó. Trong trường hợp này, file descriptor của file cần ghi dữ liệu được truyền vào hàm open() để mở file.
	// tham số đầu tiên là path là đường dẫn tới file cần ghi dữ liệu, và tham số thứ hai là O_RDONLY nghĩa là ta sẽ mở file với quyền read-only.
	// hàm này sẽ trả về file descriptor của file cần ghi dữ liệu, và nếu thất bại, hàm này sẽ trả về -1. Trong trường hợp này, nếu hàm open() trả về -1, ta sẽ in ra một thông báo lỗi và thoát chương trình.
	// nếu thành công giá trị của fd sẽ lớn hơn 0 và nhỏ hơn 1024 vì một tiến trình có thể mở tối đa 1024 file.
	const int fd = open(path, O_RDONLY); // yes, read-only! :-)
	if (fd < 0) {
		perror("open failed");
		return EXIT_FAILURE;
	}

   // khởi tạo biến st kiểu struct stat, và gán giá trị cho biến này là giá trị của hàm fstat(fd, &st). Biến st là một struct stat, và được khởi tạo bằng cách gọi hàm fstat() với tham số đầu tiên là file descriptor của file cần ghi dữ liệu, và tham số thứ hai là địa chỉ của biến st.
   // kiểu struct stat là một struct được định nghĩa trong thư viện sys/stat.h, và được sử dụng để lưu trữ thông tin về một file. Trong trường hợp này, biến st sẽ lưu trữ thông tin về file cần ghi dữ liệu.
   // biến st có vai trò lưu trữ thông tin về file cần ghi dữ liệu, và được khởi tạo bằng cách gọi hàm fstat() với tham số đầu tiên là file descriptor của file cần ghi dữ liệu, và tham số thứ hai là địa chỉ của biến st.
   // hàm fstat() sẽ lấy thông tin về file cần ghi dữ liệu, và gán giá trị của biến st bằng thông tin về file đó. Trong trường hợp này, biến st sẽ lưu trữ thông tin về file cần ghi dữ liệu.
   // hàm fstat() có tham số đầu tiên là file descriptor của file cần ghi dữ liệu, và tham số thứ hai là địa chỉ của biến st. Hàm fstat() sẽ lấy thông tin về file cần ghi dữ liệu, và gán giá trị của biến st bằng thông tin về file đó.
   // hàm fstat() sẽ trả về 0 nếu thành công, và trả về -1 nếu thất bại. Trong trường hợp này, nếu hàm fstat() trả về -1, ta sẽ in ra một thông báo lỗi và thoát chương trình.
	struct stat st;
	if (fstat(fd, &st)) {
		perror("stat failed");
		return EXIT_FAILURE;
	}

	// kiểm tra và so sánh offset với st.st_size, nếu offset lớn hơn st.st_size thì in ra một thông báo lỗi và thoát chương trình.
	if (offset > st.st_size) {
		fprintf(stderr, "Offset is not inside the file\n");
		return EXIT_FAILURE;
	}

// kiểm tra và so sánh end_offset với st.st_size, nếu end_offset lớn hơn st.st_size thì in ra một thông báo lỗi và thoát chương trình.
	if (end_offset > st.st_size) {
		fprintf(stderr, "Sorry, cannot enlarge the file\n");
		return EXIT_FAILURE;
	}

	/* create the pipe with all flags initialized with
	   PIPE_BUF_FLAG_CAN_MERGE */
	// khởi tạo biến p là một mảng hai phần tử, mỗi phần tử là một file descriptor. Biến p là một mảng hai phần tử, mỗi phần tử là một file descriptor. Mảng p này sẽ được sử dụng để tạo ra pipe.
	int p[2];
	// gọi hàm pipe(p) để tạo ra pipe. Hàm pipe() sẽ tạo ra một pipe và trả về hai file descriptor, một cho đầu đọc (read end) và một cho đầu ghi (write end) của pipe. Đầu đầu tiên của pipe được đặt tại p[0] và đầu thứ hai được đặt tại p[1]. Khi một tiến trình ghi dữ liệu vào đầu thứ hai của pipe (p[1]), tiến trình khác có thể đọc dữ liệu đó từ đầu đầu tiên của pipe (p[0]).
	prepare_pipe(p);

	/* splice one byte from before the specified offset into the
	   pipe; this will add a reference to the page cache, but
	   since copy_page_to_iter_pipe() does not initialize the
	   "flags", PIPE_BUF_FLAG_CAN_MERGE is still set */
	// khởi tạo biến offset kiểu loff_t, và gán giá trị cho biến này là offset - 1. Biến offset là vị trí bắt đầu của dữ liệu cần ghi, và được tính toán bằng cách lấy offset trừ đi 1.
	// cần trừ đi 1 của offset bởi vì ta cần lấy một byte trước vị trí bắt đầu của dữ liệu cần ghi.
	--offset;
	// gọi hàm splice(fd, &offset, p[1], NULL, 1, 0) để ghi một byte từ trước vị trí bắt đầu của dữ liệu cần ghi vào pipe. Hàm splice() sẽ ghi một byte từ file descriptor fd vào pipe, và trả về số byte đã được ghi vào pipe.
	// tham số đầu tiên của hàm splice() là file descriptor của file cần ghi dữ liệu, tham số thứ hai là địa chỉ của biến offset, tham số thứ ba là file descriptor của đầu ghi của pipe.
	// hàm splice() sẽ ghi một byte từ file descriptor fd vào pipe, và trả về số byte đã được ghi vào pipe. Trong trường hợp này, hàm splice() sẽ ghi một byte từ file descriptor fd vào pipe, và trả về số byte đã được ghi vào pipe.
	// tham số thứ ba là NULL nghĩa là ta sẽ không ghi dữ liệu vào đầu đọc của pipe. tham số thứ năm là 1 nghĩa là ta sẽ ghi một byte vào pipe. tham số thứ sáu là 0 nghĩa là ta sẽ không sử dụng các cờ cho pipe.
	// ta không sử dụng cờ trong trường hợp này là vì ta muốn thực hiện một thao tác đơn giản, và không cần quan tâm đến các cờ của pipe.
	// hàm splice() sẽ trả về số byte đã được ghi vào pipe, và nếu thất bại, hàm này sẽ trả về -1. Trong trường hợp này, nếu hàm splice() trả về -1, ta sẽ in ra một thông báo lỗi và thoát chương trình.
	// như vậy biến nbytes sẽ lưu trữ số byte đã được ghi vào pipe.Và có kiểu dữ liệu là ssize_t.
	ssize_t nbytes = splice(fd, &offset, p[1], NULL, 1, 0);
	if (nbytes < 0) {
		perror("splice failed");
		return EXIT_FAILURE;
	}
	// kiểm tra và so sánh nbytes với 0, nếu nbytes bằng 0 thì có nghĩa là hàm splice() đã ghi một byte vào pipe, nhưng không ghi được byte thứ hai vào pipe. Trong trường hợp này, ta sẽ in ra một thông báo lỗi và thoát chương trình.
	
	if (nbytes == 0) {
		fprintf(stderr, "short splice\n");
		return EXIT_FAILURE;
	}

	/* the following write will not create a new pipe_buffer, but
	   will instead write into the page cache, because of the
	   PIPE_BUF_FLAG_CAN_MERGE flag */
	// gọi hàm write(p[1], data, data_size) để ghi dữ liệu vào pipe. Hàm write() sẽ ghi dữ liệu từ một buffer vào pipe, và trả về số byte đã được ghi vào pipe.
	// ở đây ta sẽ ghi dữ liệu từ biến data vào pipe, và trả về số byte đã được ghi vào pipe. Trong trường hợp này, hàm write() sẽ ghi dữ liệu từ biến data vào pipe, và trả về số byte đã được ghi vào pipe.
	// giải thích việc gán nbytes = write(p[1], data, data_size) là vì ta muốn lưu trữ số byte đã được ghi vào pipe, và ta sẽ sử dụng số byte này để kiểm tra xem dữ liệu đã được ghi vào pipe đầy đủ hay chưa.
	// so với nbytes ở trên thì ở đây ta sẽ ghi dữ liệu từ biến data vào pipe, và trả về số byte đã được ghi vào pipe.
	
	nbytes = write(p[1], data, data_size);
	// kiểm tra nếu nbytes nhỏ hơn 0 thì in ra một thông báo lỗi và thoát chương trình.
	// bởi vì hàm write() sẽ trả về số byte đã được ghi vào pipe, và nếu thất bại, hàm này sẽ trả về -1. Trong trường hợp này, nếu hàm write() trả về -1, ta sẽ in ra một thông báo lỗi và thoát chương trình.
	if (nbytes < 0) {
		perror("write failed");
		return EXIT_FAILURE;
	}
	// kiểm tra và so sánh nbytes với data_size, nếu nbytes nhỏ hơn data_size thì in ra một thông báo lỗi và thoát chương trình.
	// bởi vì khi nbytes < data_size thì có nghĩa là hàm write() đã ghi một phần dữ liệu vào pipe, nhưng không ghi được hết dữ liệu vào pipe. Trong trường hợp này, ta sẽ in ra một thông báo lỗi và thoát chương trình.
	if ((size_t)nbytes < data_size) {
		fprintf(stderr, "short write\n");
		return EXIT_FAILURE;
	}
    
	// những trường hợp còn lại là những trường hợp thành công, ta sẽ in ra một thông báo thành công và thoát chương trình.
	printf("It worked!\n");
	return EXIT_SUCCESS;
}
