#include "sqlite3.h"
#include <iostream>
#include <string>
#include <iomanip>
#include <climits>
using namespace std;

/* Program name: Customer Rental Viewer
*  Author: Carr O'Connor
*  Date last updated: 7/18/2025
* Purpose: View customer rental information
*/

void printMainMenu();
void viewRental(sqlite3 *);
void viewCustomer(sqlite3 *);
void addRental(sqlite3 *);
int commit(sqlite3 *);
int rollback(sqlite3 *);
int mainMenu();
void printCustomerPage(sqlite3_stmt *, int, int);
void printRentalPage(sqlite3_stmt *, int, int);
	
int main()
{
	int choice;

	sqlite3 *mydb; 

    int rc = sqlite3_open("sakila.db", &mydb);
	
	cout << "Welcome to Sakila" << endl;
	choice = mainMenu();
	while (true)
	{
		switch (choice) 
		{
			case 1: viewRental(mydb); break;
			case 2: viewCustomer(mydb); break;
			case 3: addRental(mydb); break;
			case -1: return 0;
			default: cout << "That is not a valid choice." << endl;
		}
		cout << "\n\n";
		choice = mainMenu();
	}
	
} 

void printMainMenu()
{
	cout << "Please choose an option (enter -1 to quit):  " << endl;
	cout << "1. View the rentals for a customer" << endl;
	cout << "2. View Customer Information" << endl;
	cout << "3. Enter a Rental" << endl;
	cout << "Enter Choice: ";
}

int mainMenu()
{
	int choice = 0;

	printMainMenu();
	cin >> choice;
	while ((!cin || choice < 1 || choice > 3) && choice != -1)
	{
		if (!cin)
		{
			cin.clear();
			cin.ignore(INT_MAX, '\n');
		}
		cout << "That is not a valid choice." << endl
			 << endl;
		printMainMenu();
		cin >> choice;
	}
	return choice;
}

void viewRental(sqlite3 *db)
{
	string query = "SELECT customer_id, first_name, last_name FROM customer ";
	sqlite3_stmt *pRes;
	string m_strLastError;
	string query2;
	string cusID;
	string cus_fname, cus_lname;
	if (sqlite3_prepare_v2(db, query.c_str(), -1, &pRes, NULL) != SQLITE_OK)
	{
		m_strLastError = sqlite3_errmsg(db);
		sqlite3_finalize(pRes);
		cout << "There was an error: " << m_strLastError << endl;
		return;
	}
	else
	{
		int columnCount = sqlite3_column_count(pRes);
		int i = 0, choice = 0, rowsPerPage, totalRows;
		sqlite3_stmt *pRes2;
		cout << left;
		int res;
		do
		{
			res = sqlite3_step(pRes);
			i++;

		} while (res == SQLITE_ROW);
		totalRows = i - 1;
		sqlite3_reset(pRes);
		cout << "There are " << i - 1 << " rows in the result.  How many do you want to see per page?" << endl;
		cin >> rowsPerPage;
		while (!cin || rowsPerPage < 0)
		{
			if (!cin)
			{
				cin.clear();
				cin.ignore(INT_MAX, '\n');
			}
			cout << "That is not a valid choice! Try again!" << endl;
			cout << "There are " << i << " rows in the result.  How many do you want to see per page?" << endl;
		}
		if (rowsPerPage > i)
			rowsPerPage = i;
		i = 0;

		while (choice == 0 || choice == -1)
		{
			if (i == 0)
				cout << "Please choose the customer you want to see rentals for (enter 0 to go to the next page):" << endl;
			else if (i + rowsPerPage < totalRows)
				cout << "Please choose the customer you want to see rentals for (enter 0 to go to the next page or -1 to go to the previous page):" << endl;
			else
				cout << "Please choose the customer you want to see rentals for (enter -1 to go to the previous page):" << endl;
			printCustomerPage(pRes, rowsPerPage, i);
			cin >> choice;
		
			while (!(cin) || choice < -1 || choice > totalRows)
			{
				if (!cin)
				{
					cin.clear();
					cin.ignore(INT_MAX, '\n');
				}
				cout << "That is not a valid choice! Try again!" << endl;
				cin >> choice;
			}
			if (choice == 0)
			{
				i = i + rowsPerPage;

				if (i >= totalRows)
				{
					i = totalRows - rowsPerPage;
					sqlite3_reset(pRes);
					for (int j = 0; j < i; j++)
					{
						sqlite3_step(pRes);
					}
				}
			}
			else if (choice == -1)
			{
				i = i - rowsPerPage;
				if (i < 0)
					i = 0;
				sqlite3_reset(pRes);
				for (int j = 0; j < i; j++)
					sqlite3_step(pRes);
			}
		}
		sqlite3_reset(pRes);
		for (int i = 0; i < choice; i++)
			sqlite3_step(pRes);
		cusID = reinterpret_cast<const char *>(sqlite3_column_text(pRes, 0));
		cus_fname = reinterpret_cast<const char *>(sqlite3_column_text(pRes, 1));
		cus_lname = reinterpret_cast<const char *>(sqlite3_column_text(pRes, 2));
		sqlite3_finalize(pRes);
		query2 = "select rental_id, rental_date, return_date, staff.first_name || ' ' || staff.last_name as 'Staff Name', ";
		query2 += "film.title, film.description, film.rental_rate ";
		query2 += "from rental join staff on rental.staff_id = staff.staff_id ";
		query2 += "join inventory on rental.inventory_id = inventory.inventory_id ";
		query2 += "join film on film.film_id = inventory.film_id ";
		query2 += "where customer_id = " + cusID;

		if (sqlite3_prepare_v2(db, query2.c_str(), -1, &pRes2, NULL) != SQLITE_OK)
		{
			m_strLastError = sqlite3_errmsg(db);
			sqlite3_finalize(pRes2);
			cout << "There was an error: " << m_strLastError << endl;
			return;
		}
		else
		{
			columnCount = sqlite3_column_count(pRes);
			i = 0;
			choice = 0;

			do
			{
				res = sqlite3_step(pRes2);
				i++;

			} while (res == SQLITE_ROW);
			totalRows = i;
			sqlite3_reset(pRes2);
			cout << "There are " << i << " rows in the result.  How many do you want to see per page?" << endl;
			cin >> rowsPerPage;
			while (!cin || rowsPerPage < 0)
			{
				if (!cin)
				{
					cin.clear();
					cin.ignore(INT_MAX, '\n');
				}
				cout << "That is not a valid choice! Try again!" << endl;
				cout << "There are " << i << " rows in the result.  How many do you want to see per page?" << endl;
			}
			if (rowsPerPage > i)
				rowsPerPage = i;
			i = 0;

			while (choice == 0 || choice == -1)
			{
				if (i == 0)
					cout << "Please choose the rental you want to see (enter 0 to go to the next page):" << endl;
				else if (i + rowsPerPage < totalRows)
					cout << "Please choose the rental you want to see (enter 0 to go to the next page or -1 to go to the previous page):" << endl;
				else
					cout << "Please choose the rental you want to see (enter -1 to go to the previous page):" << endl;
				printRentalPage(pRes2, rowsPerPage, i);
				cin >> choice;
			
				while (!(cin) || choice < -1 || choice > totalRows)
				{
					if (!cin)
					{
						cin.clear();
						cin.ignore(INT_MAX, '\n');
					}
					cout << "That is not a valid choice! Try again!" << endl;
					cin >> choice;
				}
				if (choice == 0)
				{
					i = i + rowsPerPage;
					if (i >= totalRows)
					{
						i = totalRows - rowsPerPage;
						sqlite3_reset(pRes2);
						for (int j = 0; j < i; j++)
							sqlite3_step(pRes2);
					}
				}
				else if (choice == -1)
				{
					i = i - rowsPerPage;
					if (i < 0)
						i = 0;
					sqlite3_reset(pRes2); 
					for (int j = 0; j < i; j++)
						sqlite3_step(pRes2);
				}
			}
			sqlite3_reset(pRes2);
			for (int i = 0; i < choice; i++)
				sqlite3_step(pRes2);
		}
		string rentalID = reinterpret_cast<const char *>(sqlite3_column_text(pRes2, 0));
		string rentalDate = reinterpret_cast<const char *>(sqlite3_column_text(pRes2, 1));
		string returnDate;
		if(sqlite3_column_type(pRes2,2) != SQLITE_NULL)
			returnDate = reinterpret_cast<const char *>(sqlite3_column_text(pRes2, 2));
		else 
			returnDate = "";
		string staff = reinterpret_cast<const char *>(sqlite3_column_text(pRes2, 3));
		string filmTitle = reinterpret_cast<const char *>(sqlite3_column_text(pRes2, 4));
		string filmdescription = reinterpret_cast<const char *>(sqlite3_column_text(pRes2, 5));
		string rentalRate = reinterpret_cast<const char *>(sqlite3_column_text(pRes2, 6));
		cout << showpoint << fixed << setprecision(2);
		cout << "Rental Date: " << rentalDate << endl;
		cout << "Staff: " << staff << endl;
		cout << "Customer: " << cus_fname << " " << cus_lname << endl;
		cout << "Film Information:" << endl;
		cout << filmTitle << " - " << filmdescription << " $" << rentalRate << endl;
		cout << "Return Date: " << returnDate << endl;
		sqlite3_finalize(pRes2);
	}
}

void printCustomerPage(sqlite3_stmt *res, int rowsPerPage, int startNum)
{
	int stop, i = 1;
	do
	{
		stop = sqlite3_step(res);
		if (stop != SQLITE_ROW)
			break;
		cout << i + startNum << ". ";
		if (sqlite3_column_type(res, 0) != SQLITE_NULL)
			cout << sqlite3_column_text(res, 0) << " - ";
		if (sqlite3_column_type(res, 1) != SQLITE_NULL)
			cout << sqlite3_column_text(res, 1) << " ";
		if (sqlite3_column_type(res, 2) != SQLITE_NULL)
			cout << sqlite3_column_text(res, 2) << " ";
		cout << endl;
		i++;

	} while (i <= rowsPerPage);
}

void printRentalPage(sqlite3_stmt *res, int rowsPerPage, int startNum)
{
	int stop, i = 1;
	do
	{
		stop = sqlite3_step(res);
		if (stop != SQLITE_ROW)
			break;
		cout << i + startNum << ". ";
		if (sqlite3_column_type(res, 0) != SQLITE_NULL)
			cout << sqlite3_column_text(res, 0) << " - ";
		if (sqlite3_column_type(res, 1) != SQLITE_NULL)
			cout << sqlite3_column_text(res, 1) << " ";
		cout << endl;
		i++;

	} while (i <= rowsPerPage);
}

void viewCustomer(sqlite3 *db)
{
	// Set up sql queries and variables
	string query = "SELECT customer_id, last_name, first_name FROM customer ";
	sqlite3_stmt *pRes;
	string m_strLastError;
	string cusID;
	string cus_fname, cus_lname;
	string query2;

	// Prepare the query
	if(sqlite3_prepare_v2(db, query.c_str(), -1, &pRes, NULL) != SQLITE_OK)
	{
        m_strLastError = sqlite3_errmsg(db);
		sqlite3_finalize(pRes);
		cout << "There was an error: " << m_strLastError << endl;
		return;
	}
	else
	{
		int i = 0, choice = 0, rowsPerPage, totalRows;
		sqlite3_stmt *pRes2;
		cout << left;
		// Count total rows
		int res;
		do
		{
			res = sqlite3_step(pRes);
			i++;
		} while (res == SQLITE_ROW);
		totalRows = i - 1;
		sqlite3_reset(pRes);
		cout << "There are " << totalRows << " rows in the result.  How many do you want to see per page?" << endl;
		cin >> rowsPerPage;
		// Validate input for rows per page
		while (!cin || rowsPerPage < 0)
		{
			if (!cin)
			{
				cin.clear();
				cin.ignore(INT_MAX, '\n');
			}
			cout << "That is not a valid choice! Try again!" << endl;
			cout << "There are " << i << " rows in the result.  How many do you want to see per page?" << endl;
		}
		if (rowsPerPage > i)
			rowsPerPage = i;
		i = 0;

		// Pagination and selection loop
		while(choice == 0 || choice == -1)
		{
			if(i == 0)
				cout << "Please choose the customer you want to see rentals for (enter 0 to go to the next page):" << endl;
			else if (i + rowsPerPage < totalRows)
		        cout << "Please choose the customer you want to see rentals for (enter 0 to go to the next page or -1 to go to the previous page):" << endl;
			else
				cout << "Please choose the customer you want to see rentals for (enter -1 to go to the previous page):" << endl;
			printCustomerPage(pRes, rowsPerPage, i);
			cin >> choice;
			
			while (!(cin) || choice < -1 || choice > totalRows)
			{
				if (!cin)
				{
					cin.clear();
					cin.ignore(INT_MAX, '\n');
				}
				cout << "That is not a valid choice! Try again!" << endl;
				cin >> choice;
			}
			if (choice == 0)
			{
				i = i + rowsPerPage;

				if (i >= totalRows)
				{
					i = totalRows - rowsPerPage;

					if(i >= totalRows)
					{
						i = totalRows - rowsPerPage;
						sqlite3_reset(pRes);
						for (int j = 0; j < i; j++)
						{
							sqlite3_step(pRes);
						}
					}
				}
			}
			else if (choice == -1)
			{
				i = i - rowsPerPage;
				if (i < 0)
					i = 0;
				sqlite3_reset(pRes);
				for (int j = 0; j < i; j++)
					sqlite3_step(pRes);
			}
		}
		// Finalize the selection
		sqlite3_reset(pRes);
		for (int i = 0; i < choice; i++)
			sqlite3_step(pRes);
		// Retrieve customer information
		cusID = reinterpret_cast<const char *>(sqlite3_column_text(pRes, 0));
		cus_fname = reinterpret_cast<const char *>(sqlite3_column_text(pRes, 1));
		cus_lname = reinterpret_cast<const char *>(sqlite3_column_text(pRes, 2));
		sqlite3_finalize(pRes);
		// Build the second query to get customer details joined with address
		query2 = "SELECT a.address, a.district, a.postal_code, a.phone, c.email, c.last_update FROM customer c";
		query2 += " JOIN address a ON c.address_id = a.address_id WHERE c.customer_id = " + cusID;

		// Prepare the second query
		if(sqlite3_prepare_v2(db, query2.c_str(), -1, &pRes2, NULL) != SQLITE_OK)
		{
			m_strLastError = sqlite3_errmsg(db);
			sqlite3_finalize(pRes2);
			cout << "There was an error: " << m_strLastError << endl;
			return;
		}

		// Execute the second query and display customer information
		if (sqlite3_step(pRes2) == SQLITE_ROW)
		{
			const char *address = reinterpret_cast<const char *>(sqlite3_column_text(pRes2, 0));
			const char *district = reinterpret_cast<const char *>(sqlite3_column_text(pRes2, 1));
			const char *postalCode = reinterpret_cast<const char *>(sqlite3_column_text(pRes2, 2));
			const char *phoneNumber = reinterpret_cast<const char *>(sqlite3_column_text(pRes2, 3));
			const char *email = reinterpret_cast<const char *>(sqlite3_column_text(pRes2, 4));
			const char *lastUpdate = reinterpret_cast<const char *>(sqlite3_column_text(pRes2, 5));
			cout << "----Customer Information----" << endl;
			cout << "Name: " << cus_fname << " " << cus_lname << endl;
			cout << "Address: " << address << endl;
			cout << district << " " << postalCode << endl;
			cout << "Phone Number: " << phoneNumber << endl;
			cout << "Email: " << email << endl;
			cout << "Last Update: " << lastUpdate << endl;				
		}
		
		sqlite3_finalize(pRes2);
	}
}

// Add rental function built during class
void addRental(sqlite3 *db)
{
	int rc = sqlite3_exec(db, "begin transaction", NULL, NULL, NULL);
	if (rc != SQLITE_OK)
	{
		cout << "There was an error beginning the transaction." << endl;
		return;
	}
	string query = "SELECT customer_id, first_name, last_name FROM customer ";
	sqlite3_stmt *pRes;
	string m_strLastError;
	string query2;
	string cusID;
	string filmID;
	string staffID;
	string rentalRate;
	char date[80];
	time_t currentDate = time(NULL);
	strftime(date, 80, "%F %T", localtime(&currentDate));

	if (sqlite3_prepare_v2(db, query.c_str(), -1, &pRes, NULL) != SQLITE_OK)
	{
		m_strLastError = sqlite3_errmsg(db);
		sqlite3_finalize(pRes);
		cout << "There was an error: " << m_strLastError << endl;
		rollback(db);
		return;
	}
	else
	{
		// cout << "Please choose the customer you want to see rentals for (enter 0 to go to the next page):" << endl;
		int columnCount = sqlite3_column_count(pRes);
		int i = 0, choice = 0, rowsPerPage, totalRows;
		sqlite3_stmt *pRes2;
		cout << left;
		int res;
		do
		{
			res = sqlite3_step(pRes);
			i++;
			// cout << i << ". " << sqlite3_column_text(pRes, 0);
			// cout << endl;

		} while (res == SQLITE_ROW);
		totalRows = i - 1;
		sqlite3_reset(pRes);
		cout << "There are " << i - 1 << " rows in the result.  How many do you want to see per page?" << endl;
		cin >> rowsPerPage;
		while (!cin || rowsPerPage < 0)
		{
			if (!cin)
			{
				cin.clear();
				cin.ignore(INT_MAX, '\n');
			}
			cout << "That is not a valid choice! Try again!" << endl;
			cout << "There are " << i << " rows in the result.  How many do you want to see per page?" << endl;
		}
		if (rowsPerPage > i)
			rowsPerPage = i;
		i = 0;

		while (choice == 0 || choice == -1)
		{
			if (i == 0)
				cout << "Please choose the customer you want to see rentals for (enter 0 to go to the next page):" << endl;
			else if (i + rowsPerPage < totalRows)
				cout << "Please choose the customer you want to see rentals for (enter 0 to go to the next page or -1 to go to the previous page):" << endl;
			else
				cout << "Please choose the customer you want to see rentals for (enter -1 to go to the previous page):" << endl;
			printCustomerPage(pRes, rowsPerPage, i);
			cin >> choice;
			// Fixed a bug where going outside the bounds of the choices would NOT
			// stop the program from proceeding and would crash with a segmentation fault.
			while (!(cin) || choice < -1 || choice > totalRows)
			{
				if (!cin)
				{
					cin.clear();
					cin.ignore(INT_MAX, '\n');
				}
				cout << "That is not a valid choice! Try again!" << endl;
				cin >> choice;
			}
			if (choice == 0)
			{
				i = i + rowsPerPage;

				if (i >= totalRows)
				{
					i = totalRows - rowsPerPage;
					sqlite3_reset(pRes);
					for (int j = 0; j < i; j++)
					{
						sqlite3_step(pRes);
					}
				}
			}
			else if (choice == -1)
			{
				i = i - rowsPerPage;
				if (i < 0)
					i = 0;
				sqlite3_reset(pRes); // go back and find the right spot to display
				for (int j = 0; j < i; j++)
					sqlite3_step(pRes);
			}
		}
		sqlite3_reset(pRes);
		for (int i = 0; i < choice; i++)
			sqlite3_step(pRes);
		cusID = reinterpret_cast<const char *>(sqlite3_column_text(pRes, 0));
		sqlite3_finalize(pRes);
	}
	query = "select film.film_id, film.title, film.rental_rate, inventory_id from film join inventory on film.film_id = inventory.film_id where inventory.inventory_id not in (select inventory_id from rental where rental.return_date is null)";
	if (sqlite3_prepare_v2(db, query.c_str(), -1, &pRes, NULL) != SQLITE_OK)
	{
		m_strLastError = sqlite3_errmsg(db);
		sqlite3_finalize(pRes);
		cout << "There was an error: " << m_strLastError << endl;
		rollback(db);
		return;
	}
	else
	{
		int columnCount = sqlite3_column_count(pRes);
		int i = 0, choice = 0, rowsPerPage, totalRows;
		cout << left;
		int res;
		do
		{
			res = sqlite3_step(pRes);
			i++;
			// cout << i << ". " << sqlite3_column_text(pRes, 0);
			// cout << endl;

		} while (res == SQLITE_ROW);
		totalRows = i - 1;
		sqlite3_reset(pRes);
		cout << "There are " << i - 1 << " rows in the result.  How many do you want to see per page?" << endl;
		cin >> rowsPerPage;
		while (!cin || rowsPerPage < 0)
		{
			if (!cin)
			{
				cin.clear();
				cin.ignore(INT_MAX, '\n');
			}
			cout << "That is not a valid choice! Try again!" << endl;
			cout << "There are " << i << " rows in the result.  How many do you want to see per page?" << endl;
		}
		if (rowsPerPage > i)
			rowsPerPage = i;
		i = 0;

		while (choice == 0 || choice == -1)
		{
			if (i == 0)
				cout << "Please choose the film you want to rent (enter 0 to go to the next page):" << endl;
			else if (i + rowsPerPage < totalRows)
				cout << "Please choose the film you want to rent (enter 0 to go to the next page or -1 to go to the previous page):" << endl;
			else
				cout << "Please choose the film you want to rent (enter -1 to go to the previous page):" << endl;
			printRentalPage(pRes, rowsPerPage, i);
			cin >> choice;
			// Fixed a bug where going outside the bounds of the choices would NOT
			// stop the program from proceeding and would crash with a segmentation fault.
			while (!(cin) || choice < -1 || choice > totalRows)
			{
				if (!cin)
				{
					cin.clear();
					cin.ignore(INT_MAX, '\n');
				}
				cout << "That is not a valid choice! Try again!" << endl;
				cin >> choice;
			}
			if (choice == 0)
			{
				i = i + rowsPerPage;

				if (i >= totalRows)
				{
					i = totalRows - rowsPerPage;
					sqlite3_reset(pRes);
					for (int j = 0; j < i; j++)
					{
						sqlite3_step(pRes);
					}
				}
			}
			else if (choice == -1)
			{
				i = i - rowsPerPage;
				if (i < 0)
					i = 0;
				sqlite3_reset(pRes); // go back and find the right spot to display
				for (int j = 0; j < i; j++)
					sqlite3_step(pRes);
			}
		}
		sqlite3_reset(pRes);
		for (int i = 0; i < choice; i++)
			sqlite3_step(pRes);
		filmID = reinterpret_cast<const char *>(sqlite3_column_text(pRes, 3));
		rentalRate = reinterpret_cast<const char *>(sqlite3_column_text(pRes, 2));

		sqlite3_finalize(pRes);
	}
	query = "select staff_id, first_name || ' ' || last_name from staff";
	
	if (sqlite3_prepare_v2(db, query.c_str(), -1, &pRes, NULL) != SQLITE_OK)
	{
		m_strLastError = sqlite3_errmsg(db);
		sqlite3_finalize(pRes);
		cout << "There was an error: " << m_strLastError << endl;
		rollback(db);
		return;
	}
	else
	{
		int columnCount = sqlite3_column_count(pRes);
		int i = 0, choice = 0, rowsPerPage, totalRows;
		cout << left;
		int res;
		cout << "Please choose the staff member logging the transaction: " << endl;
		do
		{
			
			if (sqlite3_column_type(pRes, 0) != SQLITE_NULL)
			{
			cout << i << ". " << sqlite3_column_text(pRes, 0)  << " - " << sqlite3_column_text(pRes,1);
			cout << endl;
			}
			i++;
			res = sqlite3_step(pRes);

		} while (res == SQLITE_ROW);

		cin >> choice;
		while(!cin || choice < 1 || choice > i - 1)
		{
			if(!cin){
				cin.clear();
				cin.ignore(INT_MAX, '\n');
			}
			cout << "That is not a valid choice! Try again!" << endl;
			cin >> choice;
		}

		sqlite3_reset(pRes);
		for (int i = 0; i < choice; i++)
			sqlite3_step(pRes);
		staffID = reinterpret_cast<const char *>(sqlite3_column_text(pRes, 0));
		sqlite3_finalize(pRes);

	}
	string x(date);
	char * err;
	query = "insert into rental (rental_date,inventory_id, customer_id, staff_id, last_update) ";
	query += "values ('" + x + "', "+ filmID + ", " + cusID + ", " + staffID + ", '" + x + "')";
	rc = sqlite3_exec(db, query.c_str(), NULL, NULL, &err);
	if(rc != SQLITE_OK)
	{
		cout << "There was an error on insert rental: " << err << endl;
		sqlite3_free(err);
		rollback(db);
		return;
	}
	int rental_id = sqlite3_last_insert_rowid(db);
	query = "insert into payment (customer_id, staff_id, rental_id, amount, payment_date, last_update) ";
	query += "values (" + cusID + ", " + staffID + ", " + to_string(rental_id) + ", "+ rentalRate + ", '" + x + "', '" + x +"') ";
	rc = sqlite3_exec(db, query.c_str(), NULL, NULL, &err);
	if(rc != SQLITE_OK)
	{
		cout << "There was an error on insert payment: " << err << endl;
		sqlite3_free(err);
		rollback(db);
		return;
	}
	if(commit(db) == SQLITE_OK)
		cout << "Rental and Payment entered successfully. Rental Id: " << rental_id << " Payment Id: " << sqlite3_last_insert_rowid(db);
}

//Save changes
int commit(sqlite3 *db)
{
	int rc = sqlite3_exec(db, "COMMIT;", NULL, NULL, NULL);
	return rc;
}

//Discard changes
int rollback(sqlite3 *db)
{
	int rc = sqlite3_exec(db, "ROLLBACK;", NULL, NULL, NULL);
	if (rc != SQLITE_OK)
	{
		cout << "There was an error rolling back the transaction." << endl;
		return rc;
	}
	return rc;
}
