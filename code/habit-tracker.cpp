// this section includes packages 
#include "splashkit.h" 
#include "utilities.h"
#include <chrono>
#include <ctime>
#include <fstream>
#include <sstream>
#include <termios.h>
#include <unistd.h>
#include <openssl/sha.h>
#include <iomanip>  
#include <filesystem>
#include <iostream>

using std::to_string;

string get_password(const string &prompt)
{
    termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);       // Save current terminal attributes
    newt = oldt;
    newt.c_lflag &= ~(ECHO);              // Disable echo
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    string password = read_string(prompt);

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt); // Restore old terminal attributes
    write_line(); // move to next line after password input
    return password;
}


string hash_password(const string &password)
{
    unsigned char hash[SHA256_DIGEST_LENGTH]; // 32 bytes
    SHA256((unsigned char*)password.c_str(), password.size(), hash);

    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
    {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }
    return ss.str();
}


string credential_filename(const string &username)
{
    return "users/" + username + ".cred";
}

string save_filename(const string &username)
{
    return "saves/" + username + ".save";
}

struct user
{
    string correct_username;
    string correct_password;
    string user_name;
};

// this enum defines the categories of habits
enum categories
{
    HEALTH,
    FITNESS,
    STUDY,
    HOBBY,
    OTHER
};

// this struct defines the properties of a habit
struct habit
{
    string name;
    int target;
    int current_streak;
    categories category;
    bool log[28];
};

// this struct defines the application data
struct app_data
{
    habit *habits = new habit[2];
    user *users = new user[2];
    int count = 0;
    int size = 2;
    int user_count = 0;
    int user_size = 2;
};

void sign_up(app_data &data, string &active_user)
{
    user new_user;
    string password, confirm_password;

    write_line("Please enter information to continue.");

    do
    {
        new_user.correct_username = read_string("Choose a username: ");

        password = get_password("Choose a password: ");
        confirm_password = get_password("Confirm your password: ");

        if (password == confirm_password)
        {
            new_user.correct_password = hash_password(password);
            active_user = new_user.correct_username;
            break;
        }
        else
        {
            write_line("Passwords do not match. Please try again.");
        }
        write_line();
    } while (password != confirm_password);

    write("Enter your name: ");
    new_user.user_name = read_line();
    write_line("Welcome, " + new_user.user_name + "!");

    // Add the new user to the array
    if (data.user_count < data.user_size)
    {
        data.users[data.user_count] = new_user;
        data.user_count++;
    }
    else
    {
        user *newArray = new user[data.user_size * 2];
        for (int i = 0; i < data.user_count; i++)
            newArray[i] = data.users[i];
        delete[] data.users;
        data.users = newArray;
        data.user_size *= 2;
        data.users[data.user_count] = new_user;
        data.user_count++;
    }

    // Save credentials to file
    std::ofstream cred(credential_filename(new_user.correct_username));
    cred << new_user.correct_username << "\n"
         << new_user.correct_password << "\n"
         << new_user.user_name << "\n";
    cred.close();

    std::ofstream save(save_filename(new_user.correct_username));
    save.close();
}

void login(app_data &data, string &active_user)
{
    string entered_username, entered_password;

    write_line("Please log in to continue.");

    while (true)
    {
        entered_username = read_string("Username: ");
        entered_password = get_password("Password: ");

        std::ifstream cred(credential_filename(entered_username));
        if (!cred.is_open())
        {
            write_line("User not found. Please try again.");
            continue;
        }

        string stored_username, stored_hash, stored_name;
        getline(cred, stored_username);
        getline(cred, stored_hash);
        getline(cred, stored_name);
        cred.close();

        if (entered_username == stored_username && hash_password(entered_password) == stored_hash)
        {
            write_line("Login successful! Welcome back, " + stored_username + "!");
            active_user = stored_username;
            break;
        }
        else
        {
            write_line("Invalid username or password. Please try again.");
        }

        if (entered_username == "exit" || entered_password == "exit")
        {
            write_line("Exiting the application. Goodbye!");
            return;
        }
    }
}

// define the current day index
int get_today_index()
{
    std::time_t t = std::time(nullptr);
    std::tm* local_time = std::localtime(&t);

    return local_time->tm_yday % 28; // Return the day of the year modulo 28
}

// this function converts a category enum to a string representation to display to the user
string category_to_string(categories category)
{
    switch (category)
    {
    case HEALTH:
        return "Health";
    case FITNESS:
        return "Fitness";
    case STUDY:
        return "Study";
    case HOBBY:
        return "Hobby";
    case OTHER:
        return "Other";
    default:
        return "Unknown";
    }
}

// this function adds a new habit to the application data
void add_habit(app_data &data)
{

    habit new_habit; // Create a new habit instance

    // Get habit details from the user
    new_habit.name = read_string("Enter habit name: ");
    new_habit.target = read_integer("Enter target (number of days): ");

    write_line();
    write_line("Select category:");
    write_line("1: Health");
    write_line("2: Fitness");
    write_line("3: Study");
    write_line("4: Hobby");
    write_line("5: Other");
    int category_choice = read_integer("Enter your choice: ", 1, 5);
    new_habit.category = (categories)(category_choice - 1);

    // Initialize the log and current streak for the new habit
    for (int i = 0; i < 28; i++)
    {
        new_habit.log[i] = false;
    }
    new_habit.current_streak = 0;
    
    // Add the new habit to the data.habits array
    if (data.count < data.size)
    {
        data.habits[data.count] = new_habit;
        data.count++;
    }
    else // Resize the array if needed
    {
        habit *newArray = new habit[data.size * 2];
        for (int i = 0; i < data.size; i++)
        {
            newArray[i] = data.habits[i];
        }
        delete[] data.habits;
        data.habits = newArray;
        data.size *= 2;
        data.habits[data.count] = new_habit;
        data.count++;
    }

    // Confirm addition to the user
    write_line("Habit added successfully!");
}

// this function removes a habit from the application data
void remove_habit(app_data &data)
{
    // Check if there are any habits to remove
    if (data.count == 0)
    {
        write_line("No habits to remove.");
        return;
    }

    // Display current habits
    write_line("Current Habits: ");
    for (int i = 0; i < data.count; i++)
    {
        write_line(to_string(i + 1) + ". " + data.habits[i].name);
    }

    // Get the index of the habit to remove from the user
    int index = read_integer("Enter the index of the habit to remove: ") - 1;

    // Validate the index
    if (index < 0 || index >= data.count)
    {
        write_line("Invalid index.");
        return;
    }

    // Shift habits to remove the selected habit
    for (int i = index; i < data.count - 1; i++)
    {
        data.habits[i] = data.habits[i + 1];
    }
    data.count--;

    if (data.count > 0 && data.count <= data.size / 4 && data.size > 2) // Resize the array if needed
    {
        habit *newArray = new habit[data.size / 2];
        for (int i = 0; i < data.count; i++)
        {
            newArray[i] = data.habits[i];
        }
        delete[] data.habits;
        data.habits = newArray;
        data.size /= 2;
    }

    // Confirm removal to the user
    write_line("Habit removed successfully!");

}

// this function recalculates the current streak of a habit based on its log
void recalculate_streak(habit &h)
{
    int streak = 0;
    int day_index = get_today_index();

    // Count consecutive days from today backwards until a missed day is found
    for (int i = 0; i < 28; i++)
    {
        int index = (day_index - i + 28) % 28; // Wrap around using modulo
        if (h.log[index])
            streak++;
        else
            break;
    }

    //store the calculated streak in the habit
    h.current_streak = streak;
}

// this function generates a progress bar for a habit based on its current streak and target
string progress_bar(const habit &h, int bar_width = 20)
{
    // Calculate progress as a fraction of the target
    double progress = (double)h.current_streak / h.target;
    if (progress > 1.0)
    {
        progress = 1.0; // Cap at 100%
    }

    // Generate the progress bar string
    int pos = static_cast<int>(bar_width * progress);
    string bar = "[";
    for (int i = 0; i < bar_width; i++)
    {
        if (i < pos)
            bar += "#";
        else 
            bar += "-";
    }
    bar += "] ";
    int percent = static_cast<int>(progress * 100);
    bar += to_string(percent) + "%";
    
    return bar;
}

// this function generates a report of all habits in the application data
void habit_report(const app_data &data)
{
    // Check if there are any habits to report
    if (data.count == 0)
    {
        write_line("No habits to report.");
        return;
    }

    // Display the habit report
    write_line("Habit Report:");
    for (int i = 0; i < data.count; i++)
    {
        recalculate_streak(data.habits[i]);

        const habit &h = data.habits[i];
        write_line("Habit: " + h.name + ", Target: " + to_string(h.target) + ", Current Streak: " + to_string(h.current_streak) + ", Category: " + category_to_string(h.category));
        write_line(progress_bar(h));
        write_line();
    }
}

// this function checks off a habit for the current day and updates its streak
void check_habit(app_data &data)
{
    // Check if there are any habits to check off
    if (data.count == 0)
    {
        write_line("No habits to check.");
        return;
    }

    // Display current habits
    write_line("Current Habits: ");
    for (int i = 0; i < data.count; i++)
    {
        write_line(to_string(i + 1) + ". " + data.habits[i].name);
    }

    // Get the index of the habit to check off from the user
    int index = read_integer("Enter the index of the habit to check off: ") - 1;

    // Validate the index
    if (index < 0 || index >= data.count)
    {
        write_line("Invalid index.");
        return;
    }

    // Mark the habit as completed for today and update the streak
    int today_index = get_today_index();
    data.habits[index].log[today_index] = true;
    recalculate_streak(data.habits[index]);
    write_line("Habit checked off successfully!");

    // Congratulate the user if they have reached their target
    if (data.habits[index].current_streak >= data.habits[index].target)
    { 
        write_line();  
        write_line("Congratulations! You've reached your target for the habit: " + data.habits[index].name);
        write_line("After celebrating, consider setting a new target.");
        data.habits[index].target = read_integer("Enter new target (number of days): "); // Prompt for new target
    }
}

// functions to update name of a habit
void update_name(habit &data)
{
    data.name = read_string("Enter new habit name: ");
    write_line("Habit name updated successfully!");
}

// function to update target of a habit
void update_target(habit &data)
{
    data.target = read_integer("Enter new target (number of days): ");
    write_line("Habit target updated successfully!");
}

// function to update current streak of a habit
void update_streak(habit &data)
{
    data.current_streak = read_integer("Enter new current streak: ");
    do 
    {
        write_line("Invalid streak value. It must be between 0 and 28.");
        data.current_streak = read_integer("Enter new current streak: ");
    } while (data.current_streak < 0 || data.current_streak > 28);
    write_line("Habit current streak updated successfully!");
}

// function to update category of a habit
void update_category(habit &data)
{
    write_line("Select new category:");
    write_line("1. Health");
    write_line("2. Fitness");
    write_line("3. Study");
    write_line("4. Hobby");
    write_line("5. Other");
    int category_choice = read_integer("Enter your choice: ", 1, 5);
    data.category = (categories)(category_choice - 1);
    write_line("Habit category updated successfully!");
}

// this function updates the details of a habit in the application data
void update_habit(app_data &data)
{
    // Check if there are any habits to update
    if (data.count == 0)
    {
        write_line("No habits to update.");
        return;
    }

    // Display current habits
    write_line("Current Habits: ");
    for (int i = 0; i < data.count; i++)
    {
        write_line(to_string(i + 1) + ". " + data.habits[i].name);
    }

    // Get the index of the habit to update from the user
    int index = read_integer("Enter the index of the habit to update: ") - 1;

    // Validate the index
    if (index < 0 || index >= data.count)
    {
        write_line("Invalid index.");
        return;
    }

    // Display update options to the user
    write_line("Select an Option to Update:");
    write_line("1. Name");  
    write_line("2. Target");
    write_line("3. Current Streak");
    write_line("4. Category");
    write_line("5. All");
    write_line("6. Back to Main Menu");

    // Get the user's choice and perform the corresponding update
    int choice = read_integer("Enter your choice: ", 1, 6);
    while (choice != 6)
    {
        switch (choice)
        {
        case 1:
            update_name(data.habits[index]);
            break;
        case 2:
            update_target(data.habits[index]);
            break;
        case 3:
            update_streak(data.habits[index]);
            break;
        case 4:
            update_category(data.habits[index]);
            break;
        case 5:
            update_name(data.habits[index]);
            update_target(data.habits[index]);
            update_streak(data.habits[index]);
            update_category(data.habits[index]);
            break;
        default:
            write_line("Invalid choice. Please try again.");
            break;
        }
        choice = read_integer("Enter your choice: ", 1, 5);
    }
    
    // Confirm update to the user
    write_line("Habits updated successfully!");
}

// this function pauses the program until the user presses Enter
void pause_for_user()
{
    write_line();
    write_line("Press Enter to continue...");
    read_line();
}

void save_data(const app_data &data, const std::string &filename)
{
    std::ofstream file(filename);
    if (!file.is_open())
    {
        write_line("Error opening file for saving data.");
        return;
    }

    for (int i = 0; i < data.count; i++)
    {
        const habit &h = data.habits[i];
        file << h.name << ","
             << h.target << ","
             << h.current_streak << ","
             << h.category;
        for (int j = 0; j < 28; j++)
        {
            file << "," << h.log[j];
        }
        file << "\n";
    }
}

void load_data(app_data &data, const std::string &filename)
{
    std::ifstream file(filename);
    if (!file.is_open())
    {
        write_line("Error opening file for loading data.");
        return;
    }

    std::string line;
    while (std::getline(file, line))
    {
        habit h;
        std::stringstream ss(line);
        std::string token;

        std::getline(ss, h.name, ',');
        std::getline(ss, token, ',');
        h.target = std::stoi(token);
        std::getline(ss, token, ',');
        h.current_streak = std::stoi(token);
        std::getline(ss, token, ',');
        h.category = (categories)std::stoi(token);

        for (int i = 0; i < 28; i++)
        {
            std::getline(ss, token, ',');
            h.log[i] = std::stoi(token);
        }

        if (data.count < data.size)
        {
            data.habits[data.count] = h;
            data.count++;
        }
        else
        {
            habit *newArray = new habit[data.size * 2];
            for (int i = 0; i < data.size; i++)
            {
                newArray[i] = data.habits[i];
            }
            delete[] data.habits;
            data.habits = newArray;
            data.size *= 2;
            data.habits[data.count] = h;
            data.count++;
        }
    }
}

// function to handle application exit and data saving
void exit_app(app_data &data, const string &active_user)
{
    if (!active_user.empty())
    {
        save_data(data, save_filename(active_user));
        write_line("Data saved. Exiting application.");
    }
}

// this is the main function that runs the habit tracker application
int main()
{
    app_data data; // create an instance of app_data to hold the application state
    data.count = 0; // initialize habit count to 0
    std::filesystem::create_directories("users");
    std::filesystem::create_directories("saves");


    // Display welcome message and application info
    write_line("Welcome to the Habit Tracker!");
    write_line("Track your habits and stay motivated!");
    write_line("Developed by Erin");
    write_line("================================");
    write_line();
    write_line("Login or Sign Up to Continue:");
    write_line("1. Login");
    write_line("2. Sign Up");
    write_line();

      
    string active_user; // to track the currently logged-in user
    int option = read_integer("Enter your choice: ", 1, 2);
    if (option == 1)
    {
        login(data, active_user);
    }
    else
    {
        sign_up(data, active_user);
        active_user = data.users[data.user_count - 1].correct_username; // Set active user to the newly signed-up user
    }

    // load existing data from file
    load_data(data, save_filename(active_user));

    // main menu loop
    int choice;
    do
    {
        write("================================");
        write_line("\n           Main Menu:");
        write_line("================================");
        write_line("1. Add Habit");
        write_line("2. Remove Habit");
        write_line("3. Update Habit");
        write_line("4. Check Habit");
        write_line("5. View Habit Report");
        write_line("6. Exit");

        // get user choice
        choice = read_integer("Enter your choice: ", 1, 6);

        // handle user choice
        switch (choice)
        {
        case 1:
            write_line();
            add_habit(data);
            pause_for_user();
            break;
        case 2:
            write_line();
            remove_habit(data);
            pause_for_user();
            break;
        case 3:
            write_line();
            update_habit(data);
            pause_for_user();
            break;
        case 4:
            write_line();
            check_habit(data);
            pause_for_user();
            break;
        case 5:
            write_line();
            habit_report(data);
            pause_for_user();
            break;
        case 6:
            write_line();
            break;
        default:
            write_line();
            write_line("Invalid choice. Please try again.");
            break;
        }
    } while (choice != 6);

    exit_app(data, active_user); // Save data and exit
    return 0;
}