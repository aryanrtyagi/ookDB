
import arydb

# Isolated test directory. Does not touch your existing databases.
db = arydb.DatabaseManager("./arydb_debug_test")

def run(sql):
    print(f"\nSQL: {sql}")
    result = db.execute(sql)
    print("RESULT:", result)
    return result

run("CREATE DATABASE debug_db")
run("USE debug_db")

run("""
CREATE TABLE students (
    id INT,
    name VARCHAR,
    age INT,
    marks DOUBLE
)
""")

# Test 1: Single-row INSERT
run("""
INSERT INTO students
VALUES (1, 'Aryan', 22, 95.5)
""")

# Test 2: Multi-row INSERT
run("""
INSERT INTO students
VALUES (2, 'Rahul', 21, 88.5),
       (3, 'Aman', 23, 76.0)
""")

# Test 3: SELECT
run("SELECT * FROM students")

db.close()
