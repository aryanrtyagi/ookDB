
import os
import atexit
import threading

import arydb


# Resolve the AryDB project root.
PROJECT_ROOT = os.path.dirname(
    os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
)

DATABASE_PATH = os.path.join(
    PROJECT_ROOT, "data", "database.db"
)

# One database instance per process.
_db = None
_db_pid = None

# Protect database initialization and query execution.
_db_lock = threading.RLock()


def get_database():
    global _db, _db_pid

    current_pid = os.getpid()

    with _db_lock:
        # Create a new instance if one doesn't exist
        # or if this is a different process.
        if _db is None or _db_pid != current_pid:
            _db = arydb.Database(DATABASE_PATH)
            _db_pid = current_pid

        return _db


def execute_query(sql):
    with _db_lock:
        db = get_database()
        return db.execute(sql)


def close_database():
    global _db, _db_pid

    with _db_lock:
        if _db is not None:
            db = _db
            _db = None
            _db_pid = None

            db.close()


# Flush and close the database when this process exits.
atexit.register(close_database)
