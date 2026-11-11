
import { useState } from "react";
import "./App.css";
import { useDatabase } from "./hooks/useDatabase";
import QueryHistory from "./components/QueryHistory";

function App() {
  const { isConnected, loading, error, runQuery } = useDatabase();

  const [sql, setSql] = useState("");
  const [queryResult, setQueryResult] = useState(null);
  const [queryError, setQueryError] = useState("");
  const [queryHistory, setQueryHistory] = useState([]);

  const handleExecute = async () => {
    if (!sql.trim() || loading || !isConnected) return;

    const executedSql = sql.trim();

    setQueryResult(null);
    setQueryError("");

    try {
      const result = await runQuery(executedSql);

      setQueryResult(result);

      setQueryHistory((previous) => [
        {
          id: Date.now(),
          sql: executedSql,
          timestamp: new Date().toLocaleString(),
          success: true,
        },
        ...previous,
      ]);
    } catch (err) {
      setQueryError(err.message || "Query execution failed.");

      setQueryHistory((previous) => [
        {
          id: Date.now(),
          sql: executedSql,
          timestamp: new Date().toLocaleString(),
          success: false,
        },
        ...previous,
      ]);
    }
  };

  const loadPreviousQuery = (previousSql) => {
    setSql(previousSql);
  };

  const clearHistory = () => {
    setQueryHistory([]);
  };

  return (
    <div className="app">
      <header className="topbar">
        <div className="brand">
          <div className="brand-logo">A</div>
          <div>
            <h2>AryDB</h2>
            <span>DATABASE STUDIO</span>
          </div>
        </div>

        <div
          className={`connection-status ${
            isConnected ? "online" : "offline"
          }`}
        >
          <span className="status-dot"></span>
          {isConnected ? "Engine Online" : "Engine Offline"}
        </div>
      </header>

      <main className="main-layout">
        <aside className="sidebar">
          <h4>WORKSPACE</h4>

          <div className="nav-item active">
            <span>⌘</span> SQL Editor
          </div>

          <div className="nav-item">
            <span>▤</span> Tables
          </div>

          <div className="nav-item">
            <span>◷</span> Query History
          </div>

          <div className="sidebar-bottom">
            <span>ARYDB ENGINE</span>
            <p>C++ Database Engine</p>
            <p>Version 1.0</p>
          </div>
        </aside>

        <section className="workspace">
          <div className="welcome">
            <p className="eyebrow">DATABASE WORKSPACE</p>
            <h1>SQL Editor</h1>
            <p>Write and execute queries using AryDB.</p>
          </div>

          <div className="editor-panel">
            <div className="panel-header">
              <span>QUERY EDITOR</span>
              <span>SQL</span>
            </div>

            <textarea
              value={sql}
              onChange={(e) => setSql(e.target.value)}
              placeholder="Write your SQL query here..."
              spellCheck="false"
            />

            <div className="editor-footer">
              <span>
                {loading
                  ? "Executing query..."
                  : queryError || error
                    ? "Query failed"
                    : "Ready to execute"}
              </span>

              <button
                onClick={handleExecute}
                disabled={!sql.trim() || loading || !isConnected}
              >
                {loading ? "Executing..." : "▶ Execute Query"}
              </button>
            </div>
          </div>

          <div className="results-panel">
            <div className="panel-header">
              <span>QUERY RESULTS</span>
              <span>
                {queryResult
                  ? `${queryResult.rows?.length || 0} rows`
                  : "No results"}
              </span>
            </div>

            <div className="results-content">
              {queryError ? (
                <div className="query-error">
                  <h3>Query Error</h3>
                  <p>{queryError}</p>
                </div>
              ) : queryResult ? (
                <>
                  {queryResult.message && (
                    <p className="result-message">
                      {queryResult.message}
                    </p>
                  )}

                  {queryResult.columns?.length > 0 &&
                  queryResult.rows?.length > 0 ? (
                    <div className="table-wrapper">
                      <table className="result-table">
                        <thead>
                          <tr>
                            {queryResult.columns.map((column, index) => (
                              <th key={index}>{column}</th>
                            ))}
                          </tr>
                        </thead>

                        <tbody>
                          {queryResult.rows.map((row, rowIndex) => (
                            <tr key={rowIndex}>
                              {row.map((value, colIndex) => (
                                <td key={colIndex}>
                                  {value === null ? "NULL" : String(value)}
                                </td>
                              ))}
                            </tr>
                          ))}
                        </tbody>
                      </table>
                    </div>
                  ) : (
                    <div className="empty-state">
                      <h3>Query executed successfully</h3>
                      <p>
                        {queryResult.affected_rows ?? 0} rows affected.
                      </p>
                    </div>
                  )}
                </>
              ) : (
                <div className="empty-state">
                  <div className="empty-icon">⌘</div>
                  <h3>No query executed</h3>
                  <p>Your query results will appear here.</p>
                </div>
              )}
            </div>
          </div>

          <QueryHistory
            history={queryHistory}
            onSelect={loadPreviousQuery}
            onClear={clearHistory}
          />
        </section>
      </main>
    </div>
  );
}

export default App;
