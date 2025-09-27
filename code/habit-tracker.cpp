// this section includes packages 
#include "splashkit.h" 
#include "utilities.h"
#include <ctime>
#include <fstream>
#include <sstream>
#include <termios.h>
#include <unistd.h>
#include <openssl/sha.h>
#include <iomanip>  
#include <filesystem>
#include <string> 

using std::to_string;

// function to prevent the printing of the password to the terminal when a user types 
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

// function to hash password before storing in save file 
string hash_password(const string &password)
{
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char*)password.c_str(), password.size(), hash);

    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
    {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }
    return ss.str();
}

// function to return the credential file for the specific user
string credential_filename(const string &username)
{
    return "users/" + username + ".cred";
}

// function to return the save file for specific user
string save_filename(const string &username)
{
    return "saves/" + username + ".save";
}

// struct to determine variables for each user
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

// this struct defines habit
struct habit
{
    string name;
    int target;
    int current_streak;
    categories category;
    bool* log;          // dynamic array
    int log_size;        // current size of the log
    int last_day_index;  // last day the habit was updated
};

// function to determine the day of the year 
int get_today_index()
{
    std::time_t t = std::time(nullptr);
    std::tm* local_time = std::localtime(&t);
    return local_time->tm_yday; 
}

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

// this function allows a user to create a new account 
void sign_up(app_data &data, string &active_user)
{
    user new_user;
    string password, confirm_password;

    write_line("Please enter information to continue.");

    // loop until user enters valid username and password
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

    // add user name to the data
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
        {
            newArray[i] = data.users[i];
        }
        delete[] data.users;
        data.users = newArray;
        data.user_size *= 2;
        data.users[data.user_count] = new_user;
        data.user_count++;
    }

    // Save credentials to file
    std::ofstream cred(credential_filename(new_user.correct_username));
    cred << new_user.correct_username << "\n" << new_user.correct_password << "\n" << new_user.user_name << "\n";
    cred.close();

    // Create a save file for habit data
    std::ofstream save(save_filename(new_user.correct_username));
    save.close();
}

// function to allow users to login 
void login(app_data &data, string &active_user)
{
    string entered_username, entered_password;

    write_line("Please log in to continue.");

    // loop until a user exits or enters a correct username and password
    while (true)
    {
        entered_username = read_string("Username: ");
        entered_password = get_password("Password: ");

        // open and read relevant file
        std::ifstream cred(credential_filename(entered_username));
        // if failed to open, inform user
        if (!cred.is_open())
        {
            write_line("User not found. Please try again.");
            continue;
        }

        // retrieve information from the credentials file
        string stored_username, stored_hash, stored_name;
        getline(cred, stored_username);
        getline(cred, stored_hash);
        getline(cred, stored_name);
        cred.close();

        // verify username and password 
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

        // if user types exit, go back to login or signup page
        if (entered_username == "exit" || entered_password == "exit")
        {
            write_line("Exiting.");
            return;
        }
    }
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

// increase size of log if needed
void update_log_for_today(habit &h)
{
    int today_index = get_today_index();
    int days_passed = today_index - h.last_day_index;

    // create new array, transfer data, delete old array
    if (days_passed > 0)
    {
        bool* newLog = new bool[h.log_size + days_passed];
        for (int i = 0; i < h.log_size; i++)
            newLog[i] = h.log[i];
        for (int i = h.log_size; i < h.log_size + days_passed; i++)
            newLog[i] = false;

        delete[] h.log;
        h.log = newLog;
        h.log_size += days_passed;
        h.last_day_index = today_index;
    }
}

// this function adds a new habit to the application data
void add_habit(app_data &data)
{
    habit new_habit;
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

    // Initialize dynamic log for tracking habit completion 
    new_habit.log_size = 1;
    new_habit.log = new bool[new_habit.log_size]{false};
    new_habit.current_streak = 0;
    new_habit.last_day_index = get_today_index();

    // Add to data.habits (dynamic array management same as before)
    if (data.count < data.size)
    {
        data.habits[data.count] = new_habit;
        data.count++;
    }
    else
    {
        habit* newArray = new habit[data.size * 2];
        for (int i = 0; i < data.size; i++)
            newArray[i] = data.habits[i];
        delete[] data.habits;
        data.habits = newArray;
        data.size *= 2;
        data.habits[data.count] = new_habit;
        data.count++;
    }
    
    // confirm habit creation to user 
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

    // free dynamic memory allocated to the habit's log before removal 
    delete[] data.habits[index].log; 

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
    // ensures habits log is updated for the day
    update_log_for_today(h);

    int streak = 0;

    // count consecutive days completed starting from most recent day
    for (int i = h.log_size - 1; i >= 0; i--)
    {
        if (h.log[i])
        {
            streak++; // increment for each consecutive day completed
        }
        else
        {
            break; // stop counting when missed day is encountered 
        }
    }

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
        {
            bar += "#";
        }
        else 
        {
            bar += "-";
        }
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
    if (data.count == 0)
    {
        write_line("No habits to check.");
        return;
    }

    write_line("Current Habits:");
    for (int i = 0; i < data.count; i++)
    {
        write_line(to_string(i + 1) + ". " + data.habits[i].name);
    }

    int index = read_integer("Enter the index of the habit to check off: ") - 1;
    if (index < 0 || index >= data.count)
    {
        write_line("Invalid index.");
        return;
    }

    habit &h = data.habits[index];
    update_log_for_today(h);          // grow log if needed
    h.log[h.log_size - 1] = true;     // mark today as complete
    recalculate_streak(h);

    // inform user that habit was checked off successfully 
    write_line("Habit checked off successfully!");

    if (h.current_streak >= h.target)
    {
        write_line();
        write_line("Congratulations! You've reached your target for the habit: " + h.name);
        write_line("After celebrating, consider setting a new target.");
        h.target = read_integer("Enter new target (number of days): ");
    }

    if (h.current_streak == 365)
    {
        write_line();
        write_line("Congratulations! You have completed this habit for one whole year!");
        write_line("You have mastered displine!");
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
    write_line("3. Category");
    write_line("4. All");
    write_line("5. Back to Main Menu");

    // Get the user's choice and perform the corresponding update
    int choice = read_integer("Enter your choice: ", 1, 5);
    while (choice != 5)
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
            update_category(data.habits[index]);
            break;
        case 4:
            update_name(data.habits[index]);
            update_target(data.habits[index]);
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

// function to save the habit data 
void save_data(const app_data &data, const std::string &filename)
{   
    // open file
    std::ofstream file(filename);

    // if file does not open, inform user of error
    if (!file.is_open())
    {
        write_line("Error opening file for saving data.");
        return;
    }

    // seperate fields with commas and save to file
    for (int i = 0; i < data.count; i++)
    {
        habit &h = data.habits[i];
        file << h.name << "," << h.target << "," << h.current_streak << "," << h.category << "," << h.log_size << "," << h.last_day_index;
        for (int j = 0; j < h.log_size; j++)
        {
            file << "," << h.log[j];
        }
        // seperate habits by line 
        file << "\n";
    }
}

// this function reads and loads the data from a save file for a particular user
void load_data(app_data &data, const std::string &filename)
{
    // open file
    std::ifstream file(filename);

    // if there is an error opening the file, inform user
    if (!file.is_open())
    {
        write_line("Error opening file for loading data.");
        return;
    }

    // read file line by line, each line representing one habit 
    std::string line;
    while (std::getline(file, line))
    {
        std::stringstream ss(line); // parse CSV format
        habit h; // temporary habit to populate 
        std::string token;

        // read fiels in order 
        std::getline(ss, h.name, ',');
        std::getline(ss, token, ','); h.target = std::stoi(token);
        std::getline(ss, token, ','); h.current_streak = std::stoi(token);
        std::getline(ss, token, ','); h.category = (categories)std::stoi(token);
        std::getline(ss, token, ','); h.log_size = std::stoi(token);
        std::getline(ss, token, ','); h.last_day_index = std::stoi(token);

        h.log = new bool[h.log_size];
        for (int i = 0; i < h.log_size; i++)
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
            habit* newArray = new habit[data.size * 2];
            for (int i = 0; i < data.size; i++)
                newArray[i] = data.habits[i];
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

void cleanup(app_data &data)
{
    for (int i = 0; i < data.count; i++)
    {
        delete[] data.habits[i].log;
    }
    delete[] data.habits;
    delete[] data.users;
}

// this is the main function that runs the habit tracker application
int main()
{
    app_data data; // create an instance of app_data to hold the application state
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
    cleanup(data); // manage memory 
    return 0;
}