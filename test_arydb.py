import arydb

print("Connecting to AryDB...")

db = arydb.Database("data/database.db")

print("\nExecuting SQL query...")

result = db.execute("SELECT * FROM students;")

print("\nQuery Result:")
print("Columns:", result["columns"])
print("Rows:", result["rows"])
print("Message:", result["message"])
print("Success:", result["success"])

db.close()

print("\nAryDB Python integration test completed!")
