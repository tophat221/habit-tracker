// this section includes packages 
#include "splashkit.h" 
#include "utilities.h"
#include <chrono>
#include <ctime>
#include <fstream>
#include <sstream>

using std::to_string;

// define the current day index
int today_index = 0;
int get_today_index()
{
    std::time_t t = std::time(nullptr);
    std::tm* local_time = std::localtime(&t);

    return local_time->tm_yday % 28; // Return the day of the year modulo 28
}

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
    int count = 0;
    int size = 2;
};

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
    for (int i = 0; i < 0; i++)
    {
        int index = (today_index - i + 28) % 28; // Wrap around using modulo
        if (h.log[index])
            streak++;
        else
            break;
    }

    //store the calculated streak in the habit
    h.current_streak = streak;
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

// this is the main function that runs the habit tracker application
int main()
{
    app_data data; // create an instance of app_data to hold the application state
    habit h; // create an instance of habit for temporary use
    data.count = 0; // initialize habit count to 0

    load_data(data, "habits.csv");

    // Display welcome message and application info
    write_line("Welcome to the Habit Tracker!");
    write_line("Track your habits and stay motivated!");
    write_line("Developed by Erin");
    write_line("================================");
    write_line("  Day 1 of your habit journey!");

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
            write_line("Exiting the application. Goodbye!");
            break;
        default:
            write_line();
            write_line("Invalid choice. Please try again.");
            break;
        }
    } while (choice != 6);

    save_data(data, "habits.csv");
    return 0;
}