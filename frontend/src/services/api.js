
const API_BASE_URL = "/api";

export async function checkDatabaseConnection() {
  const response = await fetch(`${API_BASE_URL}/health/`);

  if (!response.ok) {
    throw new Error("Backend is unavailable");
  }

  return response.json();
}

export async function executeSQL(sql) {
  const response = await fetch(`${API_BASE_URL}/query/`, {
    method: "POST",
    headers: {
      "Content-Type": "application/json",
    },
    body: JSON.stringify({ sql }),
  });

  const result = await response.json();

  if (!response.ok) {
    throw new Error(
      result.error?.message || "Query execution failed"
    );
  }

  return result;
}
