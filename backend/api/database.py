import os
import atexit
import threading

import arydb


PROJECT_ROOT = os.path.dirname(
    os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
)

DATABASE_DIRECTORY = os.path.join(
    PROJECT_ROOT,
    "data",
    "databases"
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
        return manager.execute(sql)


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
