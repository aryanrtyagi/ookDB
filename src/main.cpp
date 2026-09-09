#include "table.h"

#include <iostream>

int main()
{
    Table table("../data/database.db");

    while (true)
    {
        std::cout << "\n";
        std::cout << "============================\n";
        std::cout << "           AryDB\n";
        std::cout << "============================\n";
        std::cout << "1. Insert\n";
        std::cout << "2. Select All\n";
        std::cout << "3. Select By ID\n";
        std::cout << "4. Update\n";
        std::cout << "5. Delete\n";
        std::cout << "6. Exit\n";
        std::cout << "============================\n";

        int choice;

        std::cout << "Enter choice: ";
        std::cin >> choice;

        if (choice == 1)
        {
            int id;
            int age;
            int salary;

            char name[32];
            char city[32];

            std::cout << "ID: ";
            std::cin >> id;

            std::cout << "Name: ";
            std::cin >> name;

            std::cout << "Age: ";
            std::cin >> age;

            std::cout << "City: ";
            std::cin >> city;

            std::cout << "Salary: ";
            std::cin >> salary;

            table.insert(
                id,
                name,
                age,
                city,
                salary);
        }
        else if (choice == 2)
        {
            table.selectAll();
        }
        else if (choice == 3)
        {
            int id;

            std::cout << "ID: ";
            std::cin >> id;

            table.selectWhere(id);
        }
        else if (choice == 4)
        {
            int id;
            int age;
            int salary;

            char name[32];
            char city[32];

            std::cout << "ID to update: ";
            std::cin >> id;

            std::cout << "New name: ";
            std::cin >> name;

            std::cout << "New age: ";
            std::cin >> age;

            std::cout << "New city: ";
            std::cin >> city;

            std::cout << "New salary: ";
            std::cin >> salary;

            table.update(
                id,
                name,
                age,
                city,
                salary);
        }
        else if (choice == 5)
        {
            int id;

            std::cout << "ID to delete: ";
            std::cin >> id;

            table.remove(id);
        }
        else if (choice == 6)
        {
            std::cout << "Closing AryDB...\n";
            break;
        }
        else
        {
            std::cout << "Invalid choice.\n";
        }
    }

    return 0;
}