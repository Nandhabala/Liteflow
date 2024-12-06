

LiteFlow:
LiteFlow is a C++ console application that allows users to manage tasks with roles such as Admin and Member. Admins can create, assign, and delete tasks, while Members can view and update task statuses. It uses MySQL for database storage and OpenSSL's SHA256 for secure password storage.

Features:
User Registration: Register as Admin or Member with a hashed password.
User Login: Log in securely with username and password.
Admin Dashboard: Create, assign, and delete tasks.
Member Dashboard: View and update task statuses.
Password Hashing: Passwords are hashed with SHA256.
Thread Safety: Mutex for safe database operations.

Requirements:
C++ Compiler (C++11 or higher)
MySQL Workbench Database
OpenSSL


Usage:
Register: Create a new user (Admin or Member).
Login: Use your credentials to log in.
Admin: Create, assign, view, or delete tasks.
Member: View tasks and update task status.

License
MIT License
