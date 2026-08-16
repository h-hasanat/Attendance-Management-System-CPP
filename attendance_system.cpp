/* Attendance Management System (Single-file C++ project)
   - Uses arrays (1D, 2D), pointers, functions, structures, file handling
   - Modules: Admin Login, Admin Student/Faculty Management, Student Login,
              Course Registration, Attendance Marking, Attendance Report,
              Leave Request, Faculty Attendance Report, Notifications,
              Attendance graph generator, UI enhancements
   - Author: (Your group) — adapt names/details as needed
*/

#include <bits/stdc++.h>
using namespace std;

/* ---------------------------
   Configuration / Constants
   --------------------------- */
const string STUDENT_FILE = "students.txt";
const string FACULTY_FILE = "faculty.txt";
const string COURSE_FILE  = "courses.txt";
const string ENROLL_FILE  = "enrollments.txt";    // studentId,courseId
const string ATTEND_FILE  = "attendance.txt";     // studentId,courseId,totalClasses,attended
const string LEAVE_FILE   = "leaves.txt";         // leaveId, studentId, courseId, date, reason, status
const string ADMIN_FILE   = "admins.txt";         // admin username/password

const int MAX_STUDENTS = 1000;
const int MAX_FACULTY  = 200;
const int MAX_COURSES  = 200;
const int MAX_COURSE_PER_STUDENT = 10;

const double REQUIRED_ATTENDANCE_PCT = 75.0; // required percentage
// Fine formula: finePerMissingPercentPerCredit * missingPercent * creditHours
const double FINE_RATE = 2.0; // monetary unit per percent missing per credit hour

/* ANSI color codes for simple coloring (works on Linux/macOS and modern Windows terminals) */
const string C_RESET = "\033[0m";
const string C_RED   = "\033[31m";
const string C_GREEN = "\033[32m";
const string C_YELLOW= "\033[33m";
const string C_BLUE  = "\033[34m";
const string C_CYAN  = "\033[36m";
const string C_WHITE = "\033[37m";

/* ---------------------------
   Data Structures (structs)
   Use structures (allowed) to group related data.
   No classes or OOP.
   --------------------------- */

struct Student {
    string id;        // unique
    string name;
    string password;
    string department;
    int year;
    // other fields can be added
};

struct Faculty {
    string id;
    string name;
    string password;
    string dept;
};

struct Course {
    string id;
    string title;
    int creditHours;
    string facultyId; // assigned faculty
};

/* Enrollment and attendance are stored in files but we will load into arrays for operations */

/* ---------------------------
   Utility functions: File IO helpers
   --------------------------- */

void pauseConsole(){
    cout << "\nPress ENTER to continue...";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

void clearScreen(){
    // Simple clear screen call (works on most terminals)
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

/* Trim helpers */
string trim(const string &s){
    size_t a = s.find_first_not_of(" \t\r\n");
    if (a == string::npos) return "";
    size_t b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b - a + 1);
}

/* Split CSV line */
vector<string> splitCSV(const string &line){
    vector<string> out;
    string cur;
    stringstream ss(line);
    while (getline(ss, cur, ',')){
        out.push_back(trim(cur));
    }
    return out;
}

/* ---------------------------
   Loading / Saving Entities
   Files are simple CSV format, comma-separated.
   --------------------------- */

void ensureFileExists(const string &filename){
    ifstream f(filename);
    if (!f.good()){
        ofstream o(filename);
        o.close();
    }
}

/* Admin handling (simple username,password in admins.txt) */
bool adminAuthenticate(const string &username, const string &password){
    ensureFileExists(ADMIN_FILE);
    ifstream fin(ADMIN_FILE);
    string line;
    while (getline(fin, line)){
        auto parts = splitCSV(line);
        if (parts.size() >= 2 && parts[0] == username && parts[1] == password){
            fin.close();
            return true;
        }
    }
    fin.close();
    return false;
}

void createAdminIfNone(){
    ensureFileExists(ADMIN_FILE);
    ifstream fin(ADMIN_FILE);
    bool has = false;
    string line;
    while (getline(fin,line)) if(trim(line).size()>0) { has = true; break; }
    fin.close();
    if (!has){
        cout << C_YELLOW << "No admin account found. Create an admin now." << C_RESET << "\n";
        string u, p;
        cout << "Enter admin username: ";
        getline(cin, u);
        cout << "Enter admin password: ";
        getline(cin, p);
        ofstream fout(ADMIN_FILE, ios::app);
        fout << trim(u) << "," << trim(p) << "\n";
        fout.close();
        cout << C_GREEN << "Admin created. Please login with these credentials." << C_RESET << "\n";
        pauseConsole();
    }
}

/* Student load/save */
vector<Student> loadStudents(){
    ensureFileExists(STUDENT_FILE);
    vector<Student> v;
    ifstream fin(STUDENT_FILE);
    string line;
    while (getline(fin, line)){
        if(trim(line).empty()) continue;
        auto p = splitCSV(line);
        Student s;
        if (p.size() >= 5){
            s.id = p[0]; s.name = p[1]; s.password = p[2]; s.department = p[3]; s.year = stoi(p[4]);
        } else {
            // backward compatible / minimal
            s.id = p.size()>0? p[0] : "";
            s.name = p.size()>1? p[1] : "";
            s.password = p.size()>2? p[2] : "";
            s.department = p.size()>3? p[3] : "";
            s.year = p.size()>4? stoi(p[4]) : 1;
        }
        v.push_back(s);
    }
    fin.close();
    return v;
}

void saveStudents(const vector<Student> &v){
    ofstream fout(STUDENT_FILE, ios::trunc);
    for (auto &s : v){
        fout << s.id << "," << s.name << "," << s.password << "," << s.department << "," << s.year << "\n";
    }
    fout.close();
}

/* Faculty load/save */
vector<Faculty> loadFaculty(){
    ensureFileExists(FACULTY_FILE);
    vector<Faculty> v;
    ifstream fin(FACULTY_FILE);
    string line;
    while (getline(fin, line)){
        if(trim(line).empty()) continue;
        auto p = splitCSV(line);
        Faculty f;
        f.id = p.size()>0? p[0] : "";
        f.name = p.size()>1? p[1] : "";
        f.password = p.size()>2? p[2] : "";
        f.dept = p.size()>3? p[3] : "";
        v.push_back(f);
    }
    fin.close();
    return v;
}

void saveFaculty(const vector<Faculty> &v){
    ofstream fout(FACULTY_FILE, ios::trunc);
    for (auto &f : v){
        fout << f.id << "," << f.name << "," << f.password << "," << f.dept << "\n";
    }
    fout.close();
}

/* Courses load/save */
vector<Course> loadCourses(){
    ensureFileExists(COURSE_FILE);
    vector<Course> v;
    ifstream fin(COURSE_FILE);
    string line;
    while (getline(fin, line)){
        if(trim(line).empty()) continue;
        auto p = splitCSV(line);
        Course c;
        c.id = p.size()>0? p[0] : "";
        c.title = p.size()>1? p[1] : "";
        c.creditHours = p.size()>2? stoi(p[2]) : 3;
        c.facultyId = p.size()>3? p[3] : "";
        v.push_back(c);
    }
    fin.close();
    return v;
}

void saveCourses(const vector<Course> &v){
    ofstream fout(COURSE_FILE, ios::trunc);
    for (auto &c : v){
        fout << c.id << "," << c.title << "," << c.creditHours << "," << c.facultyId << "\n";
    }
    fout.close();
}

/* Enrollment (studentId,courseId) */
vector<pair<string,string>> loadEnrollments(){
    ensureFileExists(ENROLL_FILE);
    vector<pair<string,string>> v;
    ifstream fin(ENROLL_FILE);
    string line;
    while (getline(fin, line)){
        if(trim(line).empty()) continue;
        auto p = splitCSV(line);
        if (p.size() >= 2) v.emplace_back(p[0], p[1]);
    }
    fin.close();
    return v;
}

void saveEnrollments(const vector<pair<string,string>> &v){
    ofstream fout(ENROLL_FILE, ios::trunc);
    for (auto &e : v){
        fout << e.first << "," << e.second << "\n";
    }
    fout.close();
}

/* Attendance file: studentId,courseId,totalClasses,attended */
struct AttendanceRow { string sid, cid; int total, present; };
vector<AttendanceRow> loadAttendance(){
    ensureFileExists(ATTEND_FILE);
    vector<AttendanceRow> v;
    ifstream fin(ATTEND_FILE);
    string line;
    while (getline(fin, line)){
        if(trim(line).empty()) continue;
        auto p = splitCSV(line);
        if (p.size() >= 4){
            AttendanceRow a; a.sid = p[0]; a.cid = p[1]; a.total = stoi(p[2]); a.present = stoi(p[3]);
            v.push_back(a);
        }
    }
    fin.close();
    return v;
}
void saveAttendance(const vector<AttendanceRow> &v){
    ofstream fout(ATTEND_FILE, ios::trunc);
    for (auto &a : v){
        fout << a.sid << "," << a.cid << "," << a.total << "," << a.present << "\n";
    }
    fout.close();
}

/* Leaves: leaveId, studentId, courseId, date, reason, status */
struct LeaveRow { string lid, sid, cid, date, reason, status; };
vector<LeaveRow> loadLeaves(){
    ensureFileExists(LEAVE_FILE);
    vector<LeaveRow> v;
    ifstream fin(LEAVE_FILE);
    string line;
    while (getline(fin, line)){
        if(trim(line).empty()) continue;
        auto p = splitCSV(line);
        LeaveRow r;
        r.lid = p.size()>0? p[0] : "";
        r.sid = p.size()>1? p[1] : "";
        r.cid = p.size()>2? p[2] : "";
        r.date = p.size()>3? p[3] : "";
        r.reason= p.size()>4? p[4] : "";
        r.status= p.size()>5? p[5] : "Pending";
        v.push_back(r);
    }
    fin.close();
    return v;
}
void saveLeaves(const vector<LeaveRow> &v){
    ofstream fout(LEAVE_FILE, ios::trunc);
    for (auto &r : v){
        fout << r.lid << "," << r.sid << "," << r.cid << "," << r.date << "," << r.reason << "," << r.status << "\n";
    }
    fout.close();
}

/* ---------------------------
   Helper search functions
   --------------------------- */
int findStudentIndexById(const vector<Student> &v, const string &id){
    for (size_t i=0;i<v.size();++i) if (v[i].id == id) return (int)i;
    return -1;
}
int findFacultyIndexById(const vector<Faculty> &v, const string &id){
    for (size_t i=0;i<v.size();++i) if (v[i].id == id) return (int)i;
    return -1;
}
int findCourseIndexById(const vector<Course> &v, const string &id){
    for (size_t i=0;i<v.size();++i) if (v[i].id == id) return (int)i;
    return -1;
}

/* ---------------------------
   UI / Boxed text helpers
   --------------------------- */
void printHeader(const string &title){
    clearScreen();
    cout << C_CYAN;
    cout << "============================================================\n";
    cout << "    " << title << "\n";
    cout << "============================================================\n";
    cout << C_RESET;
}

void printBoxed(const string &title){
    cout << C_BLUE;
    cout << "+----------------------------------------------------------+\n";
    cout << "| " << setw(54) << left << title << "|\n";
    cout << "+----------------------------------------------------------+\n";
    cout << C_RESET;
}

/* neat table print for students/courses */
void printStudentTable(const vector<Student> &students){
    cout << left << setw(12) << "StudentID" << setw(25) << "Name" << setw(12) << "Dept" << setw(6) << "Year" << "\n";
    cout << "----------------------------------------------------------------\n";
    for (auto &s : students){
        cout << setw(12) << s.id << setw(25) << s.name << setw(12) << s.department << setw(6) << s.year << "\n";
    }
}

void printCourseTable(const vector<Course> &courses){
    cout << left << setw(8) << "CourseID" << setw(30) << "Title" << setw(8) << "Credits" << setw(12) << "FacultyID" << "\n";
    cout << "----------------------------------------------------------------\n";
    for (auto &c : courses){
        cout << setw(8) << c.id << setw(30) << c.title << setw(8) << c.creditHours << setw(12) << c.facultyId << "\n";
    }
}

/* ---------------------------
   Admin: Student Management
   - Add, delete, update, view
   --------------------------- */

void adminAddStudent(vector<Student> &students){
    printHeader("Admin - Add Student");
    Student s;
    cout << "Enter Student ID: "; getline(cin, s.id);
    if (findStudentIndexById(students, s.id) != -1){
        cout << C_RED << "Student with this ID already exists." << C_RESET << "\n";
        pauseConsole(); return;
    }
    cout << "Enter Name: "; getline(cin, s.name);
    cout << "Enter Password (for student login): "; getline(cin, s.password);
    cout << "Enter Department: "; getline(cin, s.department);
    cout << "Enter Year (1-4): "; string y; getline(cin, y); s.year = stoi(y);
    students.push_back(s);
    saveStudents(students);
    cout << C_GREEN << "Student added successfully." << C_RESET << "\n";
    pauseConsole();
}

void adminViewStudents(vector<Student> &students){
    printHeader("Admin - View Students");
    if (students.empty()){
        cout << "No students found.\n";
    } else {
        printStudentTable(students);
    }
    pauseConsole();
}

void adminUpdateStudent(vector<Student> &students){
    printHeader("Admin - Update Student");
    cout << "Enter Student ID to update: ";
    string id; getline(cin, id);
    int idx = findStudentIndexById(students, id);
    if (idx == -1){
        cout << C_RED << "Student not found." << C_RESET << "\n";
        pauseConsole(); return;
    }
    Student &s = students[idx];
    cout << "Leave blank to keep existing value.\n";
    cout << "Name ["<< s.name << "]: "; string tmp; getline(cin, tmp); if (!tmp.empty()) s.name = tmp;
    cout << "Password ["<< s.password << "]: "; getline(cin,tmp); if (!tmp.empty()) s.password = tmp;
    cout << "Department ["<< s.department << "]: "; getline(cin,tmp); if (!tmp.empty()) s.department = tmp;
    cout << "Year ["<< s.year << "]: "; getline(cin,tmp); if (!tmp.empty()) s.year = stoi(tmp);
    saveStudents(students);
    cout << C_GREEN << "Student updated." << C_RESET << "\n";
    pauseConsole();
}

void adminDeleteStudent(vector<Student> &students, vector<pair<string,string>> &enrollments, vector<AttendanceRow> &attendanceRows){
    printHeader("Admin - Delete Student");
    cout << "Enter Student ID to delete: ";
    string id; getline(cin, id);
    int idx = findStudentIndexById(students, id);
    if (idx == -1){
        cout << C_RED << "Student not found." << C_RESET << "\n";
        pauseConsole(); return;
    }
    students.erase(students.begin()+idx);
    // Remove enrollments & attendance entries
    enrollments.erase(remove_if(enrollments.begin(), enrollments.end(),
        [&](const pair<string,string> &e){ return e.first == id; }), enrollments.end());
    attendanceRows.erase(remove_if(attendanceRows.begin(), attendanceRows.end(),
        [&](const AttendanceRow &a){ return a.sid == id; }), attendanceRows.end());
    saveStudents(students);
    saveEnrollments(enrollments);
    saveAttendance(attendanceRows);
    cout << C_GREEN << "Student and associated records deleted." << C_RESET << "\n";
    pauseConsole();
}

/* ---------------------------
   Admin: Faculty Management
   --------------------------- */

void adminAddFaculty(vector<Faculty> &fac){
    printHeader("Admin - Add Faculty");
    Faculty f;
    cout << "Enter Faculty ID: "; getline(cin, f.id);
    if (findFacultyIndexById(fac, f.id) != -1){ cout << C_RED << "Faculty already exists\n" << C_RESET; pauseConsole(); return; }
    cout << "Enter Name: "; getline(cin, f.name);
    cout << "Enter Password: "; getline(cin, f.password);
    cout << "Enter Department: "; getline(cin, f.dept);
    fac.push_back(f);
    saveFaculty(fac);
    cout << C_GREEN << "Faculty added.\n" << C_RESET;
    pauseConsole();
}

void adminViewFaculty(const vector<Faculty> &fac){
    printHeader("Admin - View Faculty");
    cout << left << setw(12) << "FacultyID" << setw(25) << "Name" << setw(12) << "Dept" << "\n";
    cout << "----------------------------------------------\n";
    for (auto &f : fac){
        cout << setw(12) << f.id << setw(25) << f.name << setw(12) << f.dept << "\n";
    }
    pauseConsole();
}

void adminUpdateFaculty(vector<Faculty> &fac){
    printHeader("Admin - Update Faculty");
    cout << "Enter Faculty ID to update: "; string id; getline(cin,id);
    int idx = findFacultyIndexById(fac, id);
    if (idx==-1){ cout << C_RED << "Not found\n" << C_RESET; pauseConsole(); return; }
    Faculty &f = fac[idx];
    cout << "Leave blank to keep existing.\n";
    cout << "Name ["<<f.name<<"]: "; string tmp; getline(cin,tmp); if(!tmp.empty()) f.name = tmp;
    cout << "Password ["<<f.password<<"]: "; getline(cin,tmp); if(!tmp.empty()) f.password = tmp;
    cout << "Dept ["<<f.dept<<"]: "; getline(cin,tmp); if(!tmp.empty()) f.dept = tmp;
    saveFaculty(fac);
    cout << C_GREEN << "Faculty updated.\n" << C_RESET;
    pauseConsole();
}

void adminDeleteFaculty(vector<Faculty> &fac, vector<Course> &courses){
    printHeader("Admin - Delete Faculty");
    cout << "Enter Faculty ID to delete: "; string id; getline(cin,id);
    int idx = findFacultyIndexById(fac, id);
    if (idx==-1){ cout << C_RED << "Not found\n" << C_RESET; pauseConsole(); return; }
    // Remove faculty and unassign from courses
    fac.erase(fac.begin()+idx);
    for (auto &c : courses) if (c.facultyId == id) c.facultyId = "";
    saveFaculty(fac);
    saveCourses(courses);
    cout << C_GREEN << "Faculty deleted and unassigned from courses.\n" << C_RESET;
    pauseConsole();
}

/* ---------------------------
   Course management (Admin can add courses)
   --------------------------- */

void adminAddCourse(vector<Course> &courses){
    printHeader("Admin - Add Course");
    Course c;
    cout << "Enter Course ID: "; getline(cin, c.id);
    if (findCourseIndexById(courses, c.id) != -1){ cout << C_RED << "Course exists\n" << C_RESET; pauseConsole(); return; }
    cout << "Enter Course Title: "; getline(cin, c.title);
    cout << "Enter Credit Hours (e.g., 3): "; string ch; getline(cin,ch); c.creditHours = stoi(ch);
    cout << "Assign Faculty ID (optional): "; getline(cin, c.facultyId);
    courses.push_back(c);
    saveCourses(courses);
    cout << C_GREEN << "Course added.\n" << C_RESET;
    pauseConsole();
}

void adminViewCourses(const vector<Course> &courses){
    printHeader("Admin - View Courses");
    printCourseTable((vector<Course>&)courses);
    pauseConsole();
}

/* ---------------------------
   Student operations: registration (course add/drop), view attendance, request leave
   --------------------------- */

bool studentAuthenticate(const string &sid, const string &password, vector<Student> &students){
    int idx = findStudentIndexById(students, sid);
    if (idx == -1) return false;
    return students[idx].password == password;
}

/* Helper: get courses enrolled by student */
vector<string> getStudentCourses(const vector<pair<string,string>> &enrollments, const string &sid){
    vector<string> out;
    for (auto &e : enrollments) if (e.first == sid) out.push_back(e.second);
    return out;
}

void studentRegisterCourse(const string &sid, vector<pair<string,string>> &enrollments, const vector<Course> &courses){
    printHeader("Course Registration");
    vector<string> enrolled = getStudentCourses(enrollments, sid);
    cout << "You are currently enrolled in: ";
    if (enrolled.empty()) cout << "None\n";
    else {
        for (auto &c: enrolled) cout << c << " ";
        cout << "\n";
    }
    printCourseTable((vector<Course>&)courses);
    cout << "\nEnter Course ID to add: ";
    string cid; getline(cin, cid);
    // check if exists and not already enrolled
    if (findCourseIndexById((vector<Course>&)courses, cid) == -1){
        cout << C_RED << "Course not found.\n" << C_RESET; pauseConsole(); return;
    }
    for (auto &e: enrollments) if (e.first == sid && e.second == cid){
        cout << C_YELLOW << "Already enrolled in this course.\n" << C_RESET; pauseConsole(); return;
    }
    // limit per student
    int count = 0;
    for (auto &e: enrollments) if (e.first == sid) ++count;
    if (count >= MAX_COURSE_PER_STUDENT){ cout << C_RED << "Course limit reached.\n" << C_RESET; pauseConsole(); return; }
    enrollments.emplace_back(sid, cid);
    saveEnrollments(enrollments);
    cout << C_GREEN << "Course added.\n" << C_RESET; pauseConsole();
}

void studentDropCourse(const string &sid, vector<pair<string,string>> &enrollments){
    printHeader("Drop Course");
    vector<string> enrolled = getStudentCourses(enrollments, sid);
    if (enrolled.empty()){
        cout << "You are not enrolled in any course.\n"; pauseConsole(); return;
    }
    cout << "Enrolled courses: \n";
    for (auto &c: enrolled) cout << " - " << c << "\n";
    cout << "Enter Course ID to drop: ";
    string cid; getline(cin, cid);
    auto it = find_if(enrollments.begin(), enrollments.end(),
        [&](const pair<string,string> &e){ return e.first==sid && e.second==cid; });
    if (it == enrollments.end()){
        cout << C_RED << "Not enrolled in this course.\n" << C_RESET; pauseConsole(); return;
    }
    enrollments.erase(it);
    saveEnrollments(enrollments);
    cout << C_GREEN << "Course dropped.\n" << C_RESET; pauseConsole();
}

/* Student leave request */
void studentRequestLeave(const string &sid, vector<LeaveRow> &leaves, const vector<pair<string,string>> &enrollments){
    printHeader("Leave Request");
    // list enrolled courses
    vector<string> enrolled = getStudentCourses(enrollments, sid);
    if (enrolled.empty()){
        cout << "You have no courses. Cannot request leave.\n"; pauseConsole(); return;
    }
    cout << "Your courses:\n";
    for (auto &c: enrolled) cout << " - " << c << "\n";
    string cid, date, reason;
    cout << "Enter Course ID for leave: "; getline(cin, cid);
    // check enrollment
    bool ok=false;
    for (auto &e: enrolled) if (e==cid) { ok=true; break; }
    if (!ok){ cout << C_RED << "You are not enrolled in this course.\n" << C_RESET; pauseConsole(); return; }
    cout << "Enter date (YYYY-MM-DD): "; getline(cin, date);
    cout << "Reason: "; getline(cin, reason);
    // create leave id
    string lid = "L" + to_string(time(nullptr));
    LeaveRow r; r.lid = lid; r.sid = sid; r.cid = cid; r.date = date; r.reason = reason; r.status="Pending";
    leaves.push_back(r);
    saveLeaves(leaves);
    cout << C_GREEN << "Leave request submitted (ID: " << lid << ").\n" << C_RESET;
    pauseConsole();
}

/* ---------------------------
   Faculty: mark attendance, view attendance reports, handle leaves
   --------------------------- */

bool facultyAuthenticate(const string &fid, const string &pwd, vector<Faculty> &fac){
    int idx = findFacultyIndexById(fac, fid);
    if (idx == -1) return false;
    return fac[idx].password == pwd;
}

/* Mark attendance for a course on a date
   For simplicity we increment totalClasses for that student-course row and present count if marked present.
*/
void facultyMarkAttendance(const string &fid, vector<Course> &courses, vector<pair<string,string>> &enrollments, vector<AttendanceRow> &attendanceRows){
    printHeader("Faculty - Mark Attendance");
    // list courses assigned to this faculty
    vector<string> myCourses;
    for (auto &c : courses) if (c.facultyId == fid) myCourses.push_back(c.id);
    if (myCourses.empty()){
        cout << C_YELLOW << "You have no assigned courses.\n" << C_RESET; pauseConsole(); return;
    }
    cout << "Your courses:\n";
    for (auto &cid : myCourses) cout << " - " << cid << "\n";
    cout << "Enter Course ID to mark attendance: "; string cid; getline(cin, cid);
    if (find(myCourses.begin(), myCourses.end(), cid) == myCourses.end()){
        cout << C_RED << "Course not assigned to you.\n" << C_RESET; pauseConsole(); return;
    }
    // gather students enrolled in that course
    vector<string> studentsInCourse;
    for (auto &e : enrollments) if (e.second == cid) studentsInCourse.push_back(e.first);
    if (studentsInCourse.empty()){
        cout << "No students enrolled in this course.\n"; pauseConsole(); return;
    }
    cout << "Marking attendance for course " << cid << ". Enter P for present, A for absent.\n";
    for (auto &sid : studentsInCourse){
        cout << "Student " << sid << ": ";
        string mark; getline(cin, mark);
        bool present = (!mark.empty() && (mark[0]=='P' || mark[0]=='p'));
        // find attendance row
        bool found=false;
        for (auto &row : attendanceRows){
            if (row.sid==sid && row.cid==cid){
                row.total += 1;
                if (present) row.present += 1;
                found = true;
                break;
            }
        }
        if (!found){
            AttendanceRow a; a.sid = sid; a.cid = cid; a.total = 1; a.present = present?1:0;
            attendanceRows.push_back(a);
        }
    }
    saveAttendance(attendanceRows);
    cout << C_GREEN << "Attendance marked and saved.\n" << C_RESET;
    pauseConsole();
}

/* Faculty view course attendance summary */
void facultyViewAttendanceSummary(const string &fid, vector<Course> &courses, vector<pair<string,string>> &enrollments, vector<AttendanceRow> &attendanceRows, const vector<Student> &students){
    printHeader("Faculty - Attendance Summary");
    // list courses assigned to faculty
    for (auto &c : courses){
        if (c.facultyId != fid) continue;
        cout << C_CYAN << "Course: " << c.id << " - " << c.title << " (Credits: " << c.creditHours << ")" << C_RESET << "\n";
        cout << left << setw(12) << "StudentID" << setw(25) << "Name" << setw(8) << "Total" << setw(8) << "Present" << setw(10) << "Percent" << "\n";
        cout << "--------------------------------------------------------------------------\n";
        // find enrolled students
        for (auto &e : enrollments){
            if (e.second != c.id) continue;
            string sid = e.first;
            int total=0, present=0;
            for (auto &a : attendanceRows) if (a.sid==sid && a.cid==c.id){ total=a.total; present=a.present; break; }
            double pct = (total==0?0.0: (100.0 * present / total));
            string sname = sid;
            int si = findStudentIndexById((vector<Student>&)students, sid);
            if (si!=-1) sname = students[si].name;
            cout << setw(12) << sid << setw(25) << sname << setw(8) << total << setw(8) << present << setw(9) << fixed << setprecision(2) << pct << "%\n";
        }
        cout << "\n";
    }
    pauseConsole();
}

/* ---------------------------
   Attendance Report: for student (aligned), graph, notifications & fines
   --------------------------- */

double computeAttendancePctForStudentCourse(const AttendanceRow &a){
    if (a.total == 0) return 0.0;
    return 100.0 * (double)a.present / (double)a.total;
}

/* Fine calculation: if below REQUIRED_ATTENDANCE_PCT, fine = FINE_RATE * missingPercent * creditHours */
double computeFine(double percent, int creditHours){
    if (percent >= REQUIRED_ATTENDANCE_PCT) return 0.0;
    double missing = (REQUIRED_ATTENDANCE_PCT - percent);
    return FINE_RATE * missing * creditHours;
}

/* Text-based bar chart generator */
void printAttendanceBar(double pct){
    int bars = (int)(pct / 5.0); // 20 bars = 100%
    cout << "[";
    for (int i=0;i<20;i++){
        if (i < bars) cout << "|";
        else cout << " ";
    }
    cout << "] ";
    cout << fixed << setprecision(2) << pct << "%\n";
}

void studentViewAttendanceReport(const string &sid, vector<Course> &courses, vector<pair<string,string>> &enrollments, vector<AttendanceRow> &attendanceRows){
    printHeader("Attendance Report (Student)");
    // find enrolled courses
    vector<string> enrolled = getStudentCourses(enrollments, sid);
    if (enrolled.empty()){
        cout << "You are not enrolled in any courses.\n"; pauseConsole(); return;
    }
    cout << left << setw(8) << "Course" << setw(30) << "Title" << setw(8) << "Total" << setw(8) << "Present" << setw(10) << "Percent" << setw(10) << "Fine" << "\n";
    cout << "-----------------------------------------------------------------------------------------\n";
    double totalFine = 0.0;
    for (auto &cid : enrolled){
        int idx = findCourseIndexById(courses, cid);
        string title = (idx==-1? "Unknown": courses[idx].title);
        int credit = (idx==-1? 0: courses[idx].creditHours);
        int total=0, present=0;
        for (auto &a : attendanceRows) if (a.sid==sid && a.cid==cid){ total=a.total; present=a.present; break; }
        double pct = (total==0?0.0:(100.0 * present / total));
        double fine = computeFine(pct, credit);
        totalFine += fine;
        cout << setw(8) << cid << setw(30) << title << setw(8) << total << setw(8) << present << setw(9) << fixed << setprecision(2) << pct << "%" << setw(10) << fixed << setprecision(2) << fine << "\n";
        // graph
        printAttendanceBar(pct);
    }
    cout << "\nTotal Fine Due: " << C_RED << fixed << setprecision(2) << totalFine << C_RESET << "\n";
    // Notification / warning
    for (auto &cid : enrolled){
        int idx = findCourseIndexById(courses, cid);
        int credit = (idx==-1? 0: courses[idx].creditHours);
        int total=0, present=0;
        for (auto &a : attendanceRows) if (a.sid==sid && a.cid==cid){ total=a.total; present=a.present; break; }
        double pct = (total==0?0.0:(100.0 * present / total));
        if (pct < REQUIRED_ATTENDANCE_PCT){
            cout << C_YELLOW << "Warning: Low attendance in course " << cid << ". You must maintain at least " << REQUIRED_ATTENDANCE_PCT << "%.\n" << C_RESET;
        }
    }
    pauseConsole();
}

/* ---------------------------
   Notification Module (for admin/faculty)
   - Alerts for students below attendance threshold
   --------------------------- */
void generateAttendanceWarnings(const vector<Student> &students, vector<Course> &courses, vector<pair<string,string>> &enrollments, vector<AttendanceRow> &attendanceRows){
    printHeader("Attendance Warnings (All Students)");
    for (auto &s : students){
        vector<string> myCourses = getStudentCourses(enrollments, s.id);
        for (auto &cid : myCourses){
            int total=0, present=0;
            for (auto &a : attendanceRows) if (a.sid==s.id && a.cid==cid){ total=a.total; present=a.present; break; }
            double pct = (total==0?0.0:(100.0*present/total));
            if (pct < REQUIRED_ATTENDANCE_PCT){
                cout << C_YELLOW << "Student " << s.id << " (" << s.name << ") - Course " << cid << " - Attendance: " << fixed << setprecision(2) << pct << "%\n" << C_RESET;
            }
        }
    }
    pauseConsole();
}

/* ---------------------------
   Admin Menu & Authentication flows
   --------------------------- */

void adminMenu(){
    createAdminIfNone();
    printHeader("Admin Login");
    string username, password;
    cout << "Admin Username: "; getline(cin, username);
    cout << "Admin Password: "; getline(cin, password);
    if (!adminAuthenticate(username, password)){
        cout << C_RED << "Authentication failed.\n" << C_RESET;
        pauseConsole(); return;
    }
    // load data
    vector<Student> students = loadStudents();
    vector<Faculty> faculty = loadFaculty();
    vector<Course> courses = loadCourses();
    vector<pair<string,string>> enrollments = loadEnrollments();
    vector<AttendanceRow> attendanceRows = loadAttendance();
    int choice = 0;
    do {
        printHeader("Admin Dashboard");
        printBoxed("Admin Options");
        cout << "1. Student Management\n2. Faculty Management\n3. Course Management\n4. View Enrollments\n5. Attendance Warnings\n6. Exit Admin\nChoose: ";
        string ch; getline(cin,ch);
        if (ch.empty()) ch="0";
        choice = stoi(ch);
        switch(choice){
            case 1:{
                // Student management submenu
                int sc=0;
                do{
                    printHeader("Admin - Student Management");
                    cout << "1. Add Student\n2. View Students\n3. Update Student\n4. Delete Student\n5. Back\nChoose: ";
                    string s; getline(cin,s); if (s.empty()) s="0"; sc=stoi(s);
                    if (sc==1) adminAddStudent(students);
                    else if (sc==2) adminViewStudents(students);
                    else if (sc==3) adminUpdateStudent(students);
                    else if (sc==4) adminDeleteStudent(students, enrollments, attendanceRows);
                } while (sc != 5);
                break;
            }
            case 2:{
                int fc=0;
                do {
                    printHeader("Admin - Faculty Management");
                    cout << "1. Add Faculty\n2. View Faculty\n3. Update Faculty\n4. Delete Faculty\n5. Back\nChoose: ";
                    string s; getline(cin,s); if (s.empty()) s="0"; fc = stoi(s);
                    if (fc==1) adminAddFaculty(faculty);
                    else if (fc==2) adminViewFaculty(faculty);
                    else if (fc==3) adminUpdateFaculty(faculty);
                    else if (fc==4) adminDeleteFaculty(faculty, courses);
                } while (fc != 5);
                break;
            }
            case 3:{
                int cc=0;
                do {
                    printHeader("Admin - Course Management");
                    cout << "1. Add Course\n2. View Courses\n3. Back\nChoose: ";
                    string s; getline(cin,s); if (s.empty()) s="0"; cc=stoi(s);
                    if (cc==1) adminAddCourse(courses);
                    else if (cc==2) adminViewCourses(courses);
                } while (cc!=3);
                break;
            }
            case 4:{
                printHeader("Enrollments");
                cout << left << setw(12) << "Student" << setw(12) << "Course" << "\n";
                cout << "-----------------------------\n";
                for (auto &e : enrollments) cout << setw(12) << e.first << setw(12) << e.second << "\n";
                pauseConsole();
                break;
            }
            case 5:
                generateAttendanceWarnings(students, courses, enrollments, attendanceRows);
                break;
            case 6:
                cout << C_GREEN << "Admin logout.\n" << C_RESET;
                pauseConsole();
                break;
            default:
                cout << "Invalid option.\n"; pauseConsole();
        }
        // save any changes made to global lists
        saveStudents(students);
        saveFaculty(faculty);
        saveCourses(courses);
        saveEnrollments(enrollments);
        saveAttendance(attendanceRows);
    } while (choice != 6);
}

/* ---------------------------
   Student Menu flow
   --------------------------- */

void studentMenu(){
    vector<Student> students = loadStudents();
    vector<Course> courses = loadCourses();
    vector<pair<string,string>> enrollments = loadEnrollments();
    vector<AttendanceRow> attendanceRows = loadAttendance();
    vector<LeaveRow> leaves = loadLeaves();

    printHeader("Student Login");
    cout << "Enter Student ID: "; string sid; getline(cin, sid);
    cout << "Enter Password: "; string pwd; getline(cin, pwd);
    if (!studentAuthenticate(sid, pwd, students)){
        cout << C_RED << "Invalid credentials.\n" << C_RESET; pauseConsole(); return;
    }
    int choice=0;
    do {
        printHeader("Student Dashboard");
        cout << "1. Course Registration (Add)\n2. Drop Course\n3. View Attendance Report\n4. Request Leave\n5. Logout\nChoose: ";
        string ch; getline(cin,ch); if (ch.empty()) ch="0"; choice = stoi(ch);
        switch(choice){
            case 1: studentRegisterCourse(sid, enrollments, courses); break;
            case 2: studentDropCourse(sid, enrollments); break;
            case 3: studentViewAttendanceReport(sid, courses, enrollments, attendanceRows); break;
            case 4: studentRequestLeave(sid, leaves, enrollments); break;
            case 5: cout << "Logging out...\n"; pauseConsole(); break;
            default: cout << "Invalid\n"; pauseConsole();
        }
        // persist after each loop
        saveEnrollments(enrollments);
        saveAttendance(attendanceRows);
        saveLeaves(leaves);
    } while (choice != 5);
}

/* ---------------------------
   Faculty Menu flow
   --------------------------- */

void facultyMenu(){
    vector<Faculty> faculty = loadFaculty();
    vector<Course> courses = loadCourses();
    vector<pair<string,string>> enrollments = loadEnrollments();
    vector<AttendanceRow> attendanceRows = loadAttendance();
    vector<Student> students = loadStudents();
    vector<LeaveRow> leaves = loadLeaves();

    printHeader("Faculty Login");
    cout << "Faculty ID: "; string fid; getline(cin,fid);
    cout << "Password: "; string pwd; getline(cin,pwd);
    if (!facultyAuthenticate(fid, pwd, faculty)){
        cout << C_RED << "Invalid credentials.\n" << C_RESET; pauseConsole(); return;
    }
    int choice=0;
    do {
        printHeader("Faculty Dashboard");
        cout << "1. Mark Attendance\n2. View Attendance Summary (Your Courses)\n3. View Leave Requests\n4. Logout\nChoose: ";
        string ch; getline(cin,ch); if (ch.empty()) ch="0"; choice=stoi(ch);
        switch(choice){
            case 1: facultyMarkAttendance(fid, courses, enrollments, attendanceRows); break;
            case 2: facultyViewAttendanceSummary(fid, courses, enrollments, attendanceRows, students); break;
            case 3:{
                printHeader("Leave Requests");
                for (auto &r : leaves){
                    cout << "LeaveID: " << r.lid << " Student: " << r.sid << " Course: " << r.cid << " Date: " << r.date << " Status: " << r.status << "\n";
                    cout << "Reason: " << r.reason << "\n";
                    cout << "Approve? (Y/N/Skip): ";
                    string ans; getline(cin, ans);
                    if (!ans.empty() && (ans[0]=='Y' || ans[0]=='y')) r.status = "Approved";
                    else if (!ans.empty() && (ans[0]=='N' || ans[0]=='n')) r.status = "Rejected";
                }
                saveLeaves(leaves);
                pauseConsole();
                break;
            }
            case 4: cout << "Logging out...\n"; pauseConsole(); break;
            default: cout << "Invalid\n"; pauseConsole();
        }
        saveAttendance(attendanceRows);
    } while (choice != 4);
}

/* ---------------------------
   Main: top-level menu
   --------------------------- */

void seedDemoData(){
    // only seed if no files exist or empty
    vector<Student> students = loadStudents();
    vector<Faculty> faculty = loadFaculty();
    vector<Course> courses = loadCourses();
    if (students.empty()){
        Student s1; s1.id="S1001"; s1.name="Alice"; s1.password="pass1"; s1.department="CS"; s1.year=1;
        Student s2; s2.id="S1002"; s2.name="Bob"; s2.password="pass2"; s2.department="CS"; s2.year=2;
        students.push_back(s1); students.push_back(s2);
        saveStudents(students);
    }
    if (faculty.empty()){
        Faculty f; f.id="F001"; f.name="Dr. Khan"; f.password="fac1"; f.dept="CS";
        faculty.push_back(f); saveFaculty(faculty);
    }
    if (courses.empty()){
        Course c1; c1.id="CSE101"; c1.title="Intro to Programming"; c1.creditHours=3; c1.facultyId="F001";
        Course c2; c2.id="CSE102"; c2.title="Discrete Math"; c2.creditHours=3; c2.facultyId="F001";
        courses.push_back(c1); courses.push_back(c2); saveCourses(courses);
    }
}

int mainMenu(){
    seedDemoData();
    int opt = 0;
    do {
        printHeader("Attendance Management System");
        cout << "1. Admin Login\n2. Student Login\n3. Faculty Login\n4. Exit\nChoose an option: ";
        string s; getline(cin,s); if (s.empty()) s="0"; opt = stoi(s);
        switch(opt){
            case 1: adminMenu(); break;
            case 2: studentMenu(); break;
            case 3: facultyMenu(); break;
            case 4: cout << "Exiting...\n"; break;
            default: cout << "Invalid option.\n"; pauseConsole();
        }
    } while (opt != 4);
    return 0;
}

/* Standard program entry point */
int main() {
    return mainMenu();
}
