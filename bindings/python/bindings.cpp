#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "../../src/database/database.h"
#include "../../src/record/value.h"

namespace py = pybind11;

// Convert AryDB's custom Value into a native Python object.
py::object valueToPython(const Value& value)
{
    switch (value.getType())
    {
        case DataType::INT:
            return py::int_(value.asInt());

        case DataType::DOUBLE:
            return py::float_(value.asDouble());

        case DataType::BOOLEAN:
            return py::bool_(value.asBool());

        case DataType::VARCHAR:
            return py::str(value.asString());
    }

    return py::none();
}

// Convert QueryResult into a Python-friendly object.
py::dict queryResultToPython(const QueryResult& result)
{
    py::list rows;

    for (const auto& row : result.rows)
    {
        py::list pythonRow;

        for (const auto& value : row)
        {
            pythonRow.append(valueToPython(value));
        }

        rows.append(pythonRow);
    }

    py::dict output;

    output["columns"] = result.columns;
    output["rows"] = rows;
    output["affected_rows"] = result.affectedRows;
    output["message"] = result.message;
    output["success"] = result.success;

    return output;
}

PYBIND11_MODULE(arydb, m)
{
    m.doc() = "Python bindings for AryDB Database Engine";

    py::class_<Database>(m, "Database")
        .def(py::init<const std::string&>())
        .def(
            "execute",
            [](Database& db, const std::string& sql)
            {
                QueryResult result = db.execute(sql);
                return queryResultToPython(result);
            }
        )
        .def("close", &Database::close);
}
