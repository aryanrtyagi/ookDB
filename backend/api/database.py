
import os
import sys
import atexit
import threading

# Resolve the AryDB project root.
PROJECT_ROOT = os.path.dirname(
    os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
)

# Ensure Python searches the project root before the backend directory.
if PROJECT_ROOT in sys.path:
    sys.path.remove(PROJECT_ROOT)

sys.path.insert(0, PROJECT_ROOT)

# Import the verified root-level native extension.
import arydb

DATABASE_DIRECTORY = os.path.join(
    PROJECT_ROOT,
    "arydb_runtime_test"
)

_manager = None
_manager_pid = None
_manager_lock = threading.RLock()


def get_database_manager():
    global _manager, _manager_pid

    current_pid = os.getpid()

    with _manager_lock:
        if _manager is None or _manager_pid != current_pid:
            _manager = arydb.DatabaseManager(
                DATABASE_DIRECTORY
            )
            _manager_pid = current_pid

        return _manager


def execute_query(sql):
    with _manager_lock:
        manager = get_database_manager()

        print("SQL RECEIVED:", repr(sql))
        print("DATABASE DIRECTORY:", DATABASE_DIRECTORY)
        print("NATIVE MODULE:", arydb.__file__)
        print("ACTIVE DATABASE:", manager.get_active_database_name())

        result = manager.execute(sql)

        print("ENGINE RESULT:", result)

        return result


def get_active_database_name():
    with _manager_lock:
        manager = get_database_manager()
        return manager.get_active_database_name()


def close_database():
    global _manager, _manager_pid

    with _manager_lock:
        if _manager is not None:
            manager = _manager

            _manager = None
            _manager_pid = None

            manager.close()


atexit.register(close_database)
