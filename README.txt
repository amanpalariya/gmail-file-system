Submitter name: Aman Palariya
Roll No: 2019CSB1068
Course: CS303

# What does the program do?
It creates a FUSE based filesystem. This filesystem fetches the contents from the e-mail account provided in config.txt. Using this the email-account can be used as a storage device. Like a simple version of Google Drive for non-binary data. It uses the libcurl and libfuse libraries to achieve this. The functions supported by this program are create, getattr, mkdir, rmdir, read, write, unlink, and rename. The contens of the mountpoint are fetched on the fly and not stored on the client device. Slow internet connection may result in bad performance of the program.

# How does this program work?
The user first compiles the program using the instructions as given below. After compilation of the program, the user then runs the program by providing the mountpoint and configuration file as CLI arguments. The configuration file is read and the login configuration is extracted. The, the mount_custom_fs function is called. The fuse system then is mounted to the given mountpoint after login is attempted. If the login is successful, the filesystem is created, otherwise not. 
After the filesystem has been mounted, the user can use a filebrowser to navigate the folders or use terminal commands like ls, cd, mkdir, etc. to create, remove, list folders and files.
This program uses the IMAP protocol for connection with the email server.

# How to compile and run?

1. Unzip the source code.
2. Inside the directory, create a `bulid` folder.
3. Change the working directory to `build`.
4. Inside the `build` directory, run `cmake ..`.
5. Makefiles will be generated.
6. Use `make` to compile the source code.
7. Before running, make required changes to config.txt
8. Disable security settings on your email account and enable IMAP.
9. Create a label 'Assignment5' in your email account
10. Then call `./main.out mountpoint/ ../config.txt`

## Testing

1. Unzip the source code.
2. There is a `test` directory.
3. Change the working directory to `test`.
4. Inside the `build` directory, run `cmake ..`.
5. Makefiles will be generated.
6. Use `make` to compile the source code.
7. Run `./test.out`.

# Snapshot

Compile the source code, and run

After running `ls` in the mount directory, you will see the labels in the output.
