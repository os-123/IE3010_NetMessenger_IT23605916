# Reflection

This project helped me understand how TCP client-server communication works in a practical environment.

I learned how a server can handle multiple clients using pthreads and how shared client information can be protected using a mutex. I also gained experience implementing commands for registration, broadcast messaging, private messaging, chat rooms and file sharing.

Testing the application with multiple clients helped me understand concurrency and message forwarding. Debugging the file transfer feature also helped me understand that TCP data can arrive in parts and that the receiver must handle the expected file size correctly.

I used Linux commands, GCC, Makefile and Git during the development process. Server logging was also useful for tracking registrations, messages, file transfers and client disconnections.

Overall, this assignment improved my understanding of TCP/IP network programming, client-server architecture, concurrency and practical debugging.
