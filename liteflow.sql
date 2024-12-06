create database Liteflow
use liteflow;
-- Create Users table (for user registration and login)
CREATE TABLE IF NOT EXISTS Users (
    id INT AUTO_INCREMENT PRIMARY KEY,
    username VARCHAR(50) NOT NULL UNIQUE,
    password VARCHAR(64) NOT NULL,
    role ENUM('admin', 'member') NOT NULL
);

-- Create Tasks table (for storing task information)
CREATE TABLE IF NOT EXISTS Tasks (
    task_id INT AUTO_INCREMENT PRIMARY KEY,
    task_name VARCHAR(100) NOT NULL,
    task_details TEXT NOT NULL,
    assigned_user VARCHAR(50),
    status ENUM('to-do', 'progress', 'completed') DEFAULT 'to-do',
    FOREIGN KEY (assigned_user) REFERENCES Users(username)
);

select *