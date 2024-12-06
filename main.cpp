#include <iostream>
#include <mysql.h>
#include <openssl/sha.h>
#include <mutex>
#include <thread>

using namespace std;

// Mutex for thread safety in database operations
std::mutex dbMutex;

// Function declarations
MYSQL* connectToDatabase();
string hashPassword(const string& password);
void registerUser();
void loginUser();
void adminDashboard(const string& username);
void memberDashboard(const string& username);
void createAndAssignTask();
void viewTasks();
void updateTaskStatus(const string& username);
void deleteTask();
void logout();

// Database connection function
MYSQL* connectToDatabase() {
    string server = "localhost";
    string user = "root";
    string password = "root@1234";
    string database = "liteflow";

    MYSQL* conn = mysql_init(NULL);
    if (!conn) {
        cerr << "MySQL initialization failed." << endl;
        return nullptr;
    }

    if (!mysql_real_connect(conn, server.c_str(), user.c_str(), password.c_str(), database.c_str(), 0, NULL, 0)) {
        cerr << "Connection failed: " << mysql_error(conn) << endl;
        return nullptr;
    }

    return conn;
}

// Hash password using SHA256
string hashPassword(const string& password) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char*>(password.c_str()), password.size(), hash);

    char hexStr[65];
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(hexStr + (i * 2), "%02x", hash[i]);
    }
    hexStr[64] = '\0';
    return string(hexStr);
}

// Registration function
void registerUser() {
    lock_guard<mutex> lock(dbMutex);

    string username, password, role;
    cout << "Enter Username: ";
    cin >> username;
    cout << "Enter Password: ";
    cin >> password;
    cout << "Enter Role (admin/member): ";
    cin >> role;

    // Validate role
    if (role != "admin" && role != "member") {
        cout << "Invalid role. Please enter either 'admin' or 'member'.\n";
        return;
    }

    MYSQL* conn = connectToDatabase();
    if (conn) {
        string hashedPassword = hashPassword(password);
        string query = "INSERT INTO Users (username, password, role) VALUES ('" +
                       username + "', '" + hashedPassword + "', '" + role + "')";

        if (mysql_query(conn, query.c_str()) == 0) {
            cout << "Registration successful!" << endl;
        } else {
            cerr << "Error during registration: " << mysql_error(conn) << endl;
        }

        mysql_close(conn);
    }
}

// Login function
void loginUser() {
    lock_guard<mutex> lock(dbMutex);

    string username, password;
    cout << "Enter Username: ";
    cin >> username;
    cout << "Enter Password: ";
    cin >> password;

    MYSQL* conn = connectToDatabase();
    if (conn) {
        string query = "SELECT password, role FROM Users WHERE username = '" + username + "'";
        if (mysql_query(conn, query.c_str()) == 0) {
            MYSQL_RES* res = mysql_store_result(conn);
            if (res) {
                if (mysql_num_rows(res) > 0) {
                    MYSQL_ROW row = mysql_fetch_row(res);
                    string dbPassword = row[0];
                    string role = row[1];

                    if (dbPassword == hashPassword(password)) {
                        cout << "Login successful! Welcome, " << username << ". Role: " << role << endl;
                        if (role == "admin") {
                            adminDashboard(username);
                        } else if (role == "member") {
                            memberDashboard(username);
                        }
                    } else {
                        cout << "Incorrect password." << endl;
                    }
                } else {
                    cout << "User not found." << endl;
                }
                mysql_free_result(res);
            } else {
                cerr << "Error retrieving result: " << mysql_error(conn) << endl;
            }
        } else {
            cerr << "Error during login: " << mysql_error(conn) << endl;
        }

        mysql_close(conn);
    }
}

// Admin Dashboard: Create, View, Assign Tasks
void adminDashboard(const string& username) {
    int choice;
    while (true) {
        cout << "\nAdmin Dashboard:\n";
        cout << "1. Create and Assign Task\n2. View Tasks\n3. Delete Tasks\n4. Logout";
        cout << "Enter your choice: ";
        cin >> choice;

        switch (choice) {
            case 1:
                createAndAssignTask();
                break;
            case 2:
                viewTasks();
                break;
            case 3:
                deleteTask();
                break;
            case 4:
                return;
            default:
                cout << "Invalid choice. Try again.\n";
        }
    }
}

// Member Dashboard: View and Update Tasks
void memberDashboard(const string& username) {
    int choice;
    while (true) {
        cout << "\nMember Dashboard:\n";
        cout << "1. View Tasks\n2. Update Task Status\n3. Logout\n";
        cout << "Enter your choice: ";
        cin >> choice;

        switch (choice) {
            case 1:
                viewTasks();
                break;
            case 2:
                updateTaskStatus(username);
                break;
            case 3:
                return;
            default:
                cout << "Invalid choice. Try again.\n";
        }
    }
}

// Create Task and Assign User (Admin only)
void createAndAssignTask() {
    int taskid;
    string taskName, taskDetails, username, status;
    cout << "Enter Tasks ID: ";
    cin >> taskid;
    cout << "Enter Task Name: ";
    cin.ignore();  // To clear the input buffer
    getline(cin, taskName);
    cout << "Enter Task Details: ";
    getline(cin, taskDetails);
    cout << "Enter Assigned Username: ";
    cin >> username;
    cout << "Enter Task Status (to-do, progress, completed): ";
    cin >> status;

    // Check if the username exists
    MYSQL* conn = connectToDatabase();
    if (conn) {
        string checkUserQuery = "SELECT username FROM Users WHERE username = '" + username + "'";
        if (mysql_query(conn, checkUserQuery.c_str()) == 0) {
            MYSQL_RES* res = mysql_store_result(conn);
            if (res && mysql_num_rows(res) > 0) {
                // Username exists, create and assign the task
                string query = "INSERT INTO Tasks (task_id, task_name, task_details, assigned_user, status) "
                               "VALUES (" + to_string(taskid) + ", '" + taskName + "', '" + taskDetails + "', '" + username + "', '" + status + "')";
                if (mysql_query(conn, query.c_str()) == 0) {
                    cout << "Task created and assigned to " << username << " with status '" << status << "' successfully!\n";
                } else {
                    cerr << "Error creating task: " << mysql_error(conn) << endl;
                }
            } else {
                cout << "User not found. Please ensure the username exists.\n";
            }
            mysql_free_result(res);
        } else {
            cerr << "Error checking user existence: " << mysql_error(conn) << endl;
        }
        mysql_close(conn);
    }
}

// View all tasks
void viewTasks() {
    MYSQL* conn = connectToDatabase();
    if (conn) {
        string query = "SELECT task_id, task_name, task_details, assigned_user, status FROM Tasks";
        if (mysql_query(conn, query.c_str()) == 0) {
            MYSQL_RES* res = mysql_store_result(conn);
            if (res) {
                MYSQL_ROW row;
                while ((row = mysql_fetch_row(res))) {
                    cout << "Task ID: " << row[0] << "\nTask Name: " << row[1] << "\nTask Details: " << row[2]
                         << "\nAssigned User: " << row[3] << "\nStatus: " << row[4] << "\n\n";
                }
                mysql_free_result(res);
            } else {
                cerr << "Error retrieving tasks: " << mysql_error(conn) << endl;
            }
        } else {
            cerr << "Error querying tasks: " << mysql_error(conn) << endl;
        }
        mysql_close(conn);
    }
}

// Update task status (Member only)
void updateTaskStatus(const string& username) {
    int taskId;
    string status;
    cout << "Enter Task ID to update status: ";
    cin >> taskId;
    cout << "Enter new status (to-do, progress, completed): ";
    cin >> status;

    MYSQL* conn = connectToDatabase();
    if (conn) {
        string query = "SELECT * FROM Tasks WHERE task_id = " + to_string(taskId) + " AND assigned_user = '" + username + "'";
        if (mysql_query(conn, query.c_str()) == 0) {
            MYSQL_RES* res = mysql_store_result(conn);
            if (res && mysql_num_rows(res) > 0) {
                // User is assigned to the task, update the status
                query = "UPDATE Tasks SET status = '" + status + "' WHERE task_id = " + to_string(taskId);
                if (mysql_query(conn, query.c_str()) == 0) {
                    cout << "Task status updated successfully!\n";
                } else {
                    cerr << "Error updating task status: " << mysql_error(conn) << endl;
                }
            } else {
                cout << "Task not found or you are not assigned to this task.\n";
            }
            mysql_free_result(res);
        } else {
            cerr << "Error checking task: " << mysql_error(conn) << endl;
        }
        mysql_close(conn);
    }
}
// Delete Task (Admin only)
void deleteTask() {
    int taskId;
    cout << "Enter Task ID to delete: ";
    cin >> taskId;

    MYSQL* conn = connectToDatabase();
    if (conn) {
        // Check if task exists
        string query = "SELECT * FROM Tasks WHERE task_id = " + to_string(taskId);
        if (mysql_query(conn, query.c_str()) == 0) {
            MYSQL_RES* res = mysql_store_result(conn);
            if (res && mysql_num_rows(res) > 0) {
                // Task exists, proceed with deletion
                query = "DELETE FROM Tasks WHERE task_id = " + to_string(taskId);
                if (mysql_query(conn, query.c_str()) == 0) {
                    cout << "Task with ID " << taskId << " has been deleted successfully.\n";
                } else {
                    cerr << "Error deleting task: " << mysql_error(conn) << endl;
                }
            } else {
                cout << "Task with ID " << taskId << " not found.\n";
            }
            mysql_free_result(res);
        } else {
            cerr << "Error checking task existence: " << mysql_error(conn) << endl;
        }
        mysql_close(conn);
    }
}

// Logout function
void logout() {
    cout << "Logging out...\n";
}

// Main function
int main() {
    int choice;
    while (true) {
        cout << "\nWelcome to LiteFlow Task Manager\n";
        cout << "1. Register\n2. Login\n3. Exit\n";
        cout << "Enter your choice: ";
        cin >> choice;

        switch (choice) {
            case 1:
                registerUser();
                break;
            case 2:
                loginUser();
                break;
            case 3:
                exit(0);
            default:
                cout << "Invalid choice. Try again.\n";
        }
    }
}
