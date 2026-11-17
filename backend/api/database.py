
import os
import sys
import atexit
import threading

PROJECT_ROOT = os.path.dirname(
    os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
)

BUILD_DIRECTORY = os.path.join(PROJECT_ROOT, "build")

# Prefer the freshly built project-root extension locally.
# If it is absent, Python can still find the installed Linux extension.
for path in (BUILD_DIRECTORY, PROJECT_ROOT):
    if path in sys.path:
        sys.path.remove(path)

sys.path.insert(0, BUILD_DIRECTORY)
sys.path.insert(0, PROJECT_ROOT)

import arydb


# Local default; Render will supply ARYDB_DATA_DIR.
DATABASE_DIRECTORY = os.getenv(
    "ARYDB_DATA_DIR",
    os.path.join(PROJECT_ROOT, "arydb_runtime_test"),
)

os.makedirs(DATABASE_DIRECTORY, exist_ok=True)

_manager = None
_manager_pid = None
_manager_lock = threading.RLock()


def get_database_manager():
    global _manager, _manager_pid

    current_pid = os.getpid()

    with _manager_lock:
        if _manager is None or _manager_pid != current_pid:
            _manager = arydb.DatabaseManager(DATABASE_DIRECTORY)
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
