# Reflection

This project helped me develop a better understanding of TCP/IP network programming and practical client-server communication. Before starting this assignment, I had mainly studied networking concepts theoretically. Through this project, I was able to apply those concepts by developing a multi-client chat and file-sharing application using C.

One of the main things I learned was how a TCP server can communicate with multiple clients at the same time. I used pthreads to create a separate thread for each connected client. I also learned why shared client information needs proper synchronization and how a mutex can help protect shared data. Implementing registration, user presence, broadcasting, private messaging, chat rooms and graceful disconnection helped me understand how different client-server features work together.

The file-sharing feature was another important learning experience. I learned that file data must be handled carefully when transferring data through TCP. Testing the file transfer with different clients helped me understand the importance of correctly handling the file size and storing received files in the required directory structure.

During development, I faced several challenges while debugging the server and client programs. Testing the application with multiple clients helped me identify communication and message-forwarding issues. Server logging was also useful because it allowed me to record important events such as registrations, messages, file transfers and client disconnections. This made troubleshooting easier.

I also improved my practical knowledge of Linux commands, GCC, Makefile, Git and GitHub. Using Git commits throughout the project helped me understand how version control can be used to track development progress. Setting up SSH authentication also gave me practical experience with secure GitHub access.

Overall, this assignment improved my understanding of TCP/IP programming, client-server architecture, concurrency, synchronization, file transfer, debugging and version control. It also improved my confidence in developing and testing a network application in a Linux environment.
