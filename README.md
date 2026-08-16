# Attendance Management System (C++)

A console-based **Attendance Management System** written in C++ for managing students, faculty, courses, enrollments, attendance, and leave requests through role-based menus.

## Features

### Admin
- Admin login
- Add, update, view, and delete student records
- Add, update, view, and delete faculty records
- Create and manage courses
- Assign faculty to courses

### Student
- Student login
- Register for courses
- Drop registered courses
- View attendance reports
- View attendance percentage and text-based attendance graph
- Receive a warning when attendance falls below 75%
- Submit leave requests

### Faculty
- Faculty login
- Mark student attendance
- View attendance summaries for assigned courses
- Review and approve/reject student leave requests

### Data Management
The application stores data locally using simple comma-separated text files:
- `students.txt`
- `faculty.txt`
- `courses.txt`
- `enrollments.txt`
- `attendance.txt`
- `leaves.txt`
- `admins.txt`

These files are created automatically when the application runs.

## Technologies & Concepts
- C++
- Structures and functions
- Vectors and arrays
- File handling / persistent storage
- CRUD operations
- Role-based application flow
- Input validation
- Attendance calculations
- Modular procedural programming

## Requirements

A C++17-compatible compiler such as GCC/MinGW or Clang.

## Build and Run

### Linux / macOS
```bash
g++ -std=c++17 attendance_system.cpp -o attendance_system
./attendance_system
```

### Windows (MinGW)
```powershell
g++ -std=c++17 attendance_system.cpp -o attendance_system.exe
.\attendance_system.exe
```

## Demo Data

On first run, the program can seed example student, faculty, and course records for demonstration purposes.

> **Note:** This is an academic/demo project. Credentials are stored as plain text and the login system should not be considered production-grade security.

## Project Purpose

This project was developed to practice core C++ programming concepts by applying them to a practical university attendance workflow involving multiple user roles, persistent data, attendance calculations, course registration, and leave management.

## Author

**Hasher Hasanat**  
BS Computer Science — Bahria University  
GitHub: [h-hasanat](https://github.com/h-hasanat)  
LinkedIn: [h-hasanat](https://www.linkedin.com/in/h-hasanat/)
