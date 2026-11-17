import { useMemo, useState } from "react";
import "./App.css";
import { useDatabase } from "./hooks/useDatabase";

function formatExecutionTime(startedAt, finishedAt) {
  return `${((finishedAt - startedAt) / 1000).toFixed(3)} s`;
}

function QueryOutput({ entry }) {
  const result = entry.result;

  return (
    <section className="workbench-entry">
      <div className="workbench-command">
        <span className="terminal-prompt">arydb&gt;</span>
        <pre className="terminal-sql">{entry.sql}</pre>
      </div>

      {entry.success ? (
        <div className="workbench-response success-response">
          <div className="response-status">
            <span className="response-icon">✓</span>
            <span>{result?.message || "Query executed successfully."}</span>
            <span className="response-time">{entry.executionTime}</span>
          </div>

          {result?.columns?.length > 0 && result?.rows?.length > 0 ? (
            <div className="workbench-result-table-wrap">
              <table className="workbench-result-table">
                <thead>
                  <tr>
                    {result.columns.map((column, index) => (
                      <th key={`${column}-${index}`}>{column}</th>
                    ))}
                  </tr>
                </thead>
                <tbody>
                  {result.rows.map((row, rowIndex) => (
                    <tr key={rowIndex}>
                      {row.map((value, columnIndex) => (
                        <td key={columnIndex} className={value === null ? "null-value" : ""}>
                          {value === null ? "NULL" : String(value)}
                        </td>
                      ))}
                    </tr>
                  ))}
                </tbody>
              </table>
              <div className="result-row-count">
                {result.rows.length} {result.rows.length === 1 ? "row" : "rows"} returned
              </div>
            </div>
          ) : (
            <div className="response-detail">
              {result?.affected_rows ?? 0} rows affected.
            </div>
          )}
        </div>
      ) : (
        <div className="workbench-response error-response">
          <div className="response-status">
            <span className="response-icon">×</span>
            <strong>Error:</strong>
            <span>{entry.error}</span>
            <span className="response-time">{entry.executionTime}</span>
          </div>
        </div>
      )}
    </section>
  );
}

function App() {
  const { isConnected, loading, error, runQuery, checkConnection } = useDatabase();

  const [sql, setSql] = useState("");
  const [entries, setEntries] = useState([]);
  const [leftOpen, setLeftOpen] = useState(true);
  const [rightOpen, setRightOpen] = useState(true);
  const [showHistory, setShowHistory] = useState(false);
  const [selectedTable, setSelectedTable] = useState("");

  const latestStatus = useMemo(() => {
    if (loading) return "Executing query...";
    if (!isConnected) return error || "Engine offline";
    return "Ready";
  }, [loading, isConnected, error]);

  const executeQuery = async () => {
    const executedSql = sql.trim();
    if (!executedSql || loading || !isConnected) return;

    const startedAt = performance.now();
    setSql("");

    try {
      const result = await runQuery(executedSql);
      const finishedAt = performance.now();
      setEntries((current) => [
        ...current,
        {
          id: `${Date.now()}-${current.length}`,
          sql: executedSql,
          result,
          success: true,
          executionTime: formatExecutionTime(startedAt, finishedAt),
          timestamp: new Date().toLocaleTimeString(),
        },
      ]);
    } catch (queryError) {
      const finishedAt = performance.now();
      setEntries((current) => [
        ...current,
        {
          id: `${Date.now()}-${current.length}`,
          sql: executedSql,
          error: queryError.message || "Query execution failed.",
          success: false,
          executionTime: formatExecutionTime(startedAt, finishedAt),
          timestamp: new Date().toLocaleTimeString(),
        },
      ]);
    }
  };

  const handleEditorKeyDown = (event) => {
    if (event.key === "Enter" && !event.shiftKey) {
      event.preventDefault();
      executeQuery();
    }
  };

  const loadHistoryEntry = (entry) => {
    setSql(entry.sql);
    setShowHistory(false);
  };

  return (
    <div className="app arydb-app">
      <header className="topbar">
        <div className="brand">
          <div className="brand-logo">A</div>
          <div className="brand-copy">
            <h2>AryDB Studio</h2>
            <span>Custom Relational Database Engine</span>
          </div>
        </div>

        <div className="topbar-center">SQL Workbench</div>

        <div className={`connection-status ${isConnected ? "online" : "offline"}`}>
          <span className="status-dot" />
          {isConnected ? "Engine Online" : "Engine Offline"}
        </div>
      </header>

      <main className={`main-layout workbench-layout ${!leftOpen ? "left-collapsed" : ""} ${!rightOpen ? "right-collapsed" : ""}`}>
        {leftOpen && (
          <aside className="explorer-panel side-panel">
            <div className="side-panel-heading">
              <h3>Database Explorer</h3>
              <button className="icon-button" onClick={checkConnection} title="Refresh connection" aria-label="Refresh connection">↻</button>
            </div>
            <div className="explorer-search">
              <span>⌕</span>
              <span>Search databases or tables</span>
            </div>
            <div className="panel-empty-state">
              <span className="panel-empty-icon">▤</span>
              <strong>Database explorer</strong>
              <p>Database and table navigation will appear here once the explorer API is connected.</p>
            </div>
          </aside>
        )}

        <section className="workspace workbench-workspace">
          <div className="workbench-toolbar">
            <div className="toolbar-left">
              <button className="icon-button panel-toggle" onClick={() => setLeftOpen((open) => !open)} title={leftOpen ? "Hide database explorer" : "Show database explorer"} aria-label="Toggle database explorer">{leftOpen ? "◧" : "▤"}</button>
              <span className="workbench-title">SQL Workbench</span>
              <button className="icon-button panel-toggle" onClick={() => setRightOpen((open) => !open)} title={rightOpen ? "Hide table inspector" : "Show table inspector"} aria-label="Toggle table inspector">{rightOpen ? "◨" : "▣"}</button>
            </div>
            <div className="toolbar-actions">
              <button className="toolbar-button" onClick={() => setEntries([])} disabled={entries.length === 0}>Clear</button>
              <button className={`toolbar-button ${showHistory ? "active" : ""}`} onClick={() => setShowHistory((visible) => !visible)}>History ({entries.length})</button>
            </div>
          </div>

          {showHistory && (
            <div className="history-popover">
              <div className="history-popover-heading">
                <strong>Query History</strong>
                <button className="icon-button" onClick={() => setShowHistory(false)} aria-label="Close history">×</button>
              </div>
              {entries.length === 0 ? (
                <p className="history-empty-message">No executed queries yet.</p>
              ) : (
                <div className="history-popover-list">
                  {[...entries].reverse().map((entry) => (
                    <button className="history-popover-item" key={entry.id} onClick={() => loadHistoryEntry(entry)}>
                      <span className={entry.success ? "history-success-mark" : "history-error-mark"}>{entry.success ? "✓" : "×"}</span>
                      <span className="history-query-text">{entry.sql.replace(/\s+/g, " ")}</span>
                      <span className="history-time-text">{entry.timestamp}</span>
                    </button>
                  ))}
                </div>
              )}
            </div>
          )}

          <div className="terminal-panel" aria-label="AryDB SQL workbench">
            <div className="terminal-intro">
              <div>AryDB Interactive SQL Workbench</div>
              <div>Engine status: <span className={isConnected ? "terminal-online" : "terminal-offline"}>{isConnected ? "Connected" : "Disconnected"}</span></div>
            </div>

            <div className="terminal-output" aria-live="polite">
              {entries.map((entry) => <QueryOutput entry={entry} key={entry.id} />)}
            </div>

            <div className="terminal-input-row">
              <span className="terminal-prompt">arydb&gt;</span>
              <textarea
                className="terminal-input"
                value={sql}
                onChange={(event) => setSql(event.target.value)}
                onKeyDown={handleEditorKeyDown}
                rows={1}
                spellCheck="false"
                aria-label="SQL command input"
                disabled={!isConnected || loading}
              />
              {loading && <span className="terminal-running">Running...</span>}
            </div>
          </div>
          <div className="workbench-statusbar">
            <span className={isConnected ? "status-online-text" : "status-offline-text"}>● {latestStatus}</span>
            <span>Enter: execute · Shift+Enter: new line</span>
          </div>
        </section>

        {rightOpen && (
          <aside className="inspector-panel side-panel">
            <div className="side-panel-heading">
              <h3>Table Inspector</h3>
              <button className="icon-button" onClick={() => setSelectedTable("")} title="Clear selected table" aria-label="Clear selected table">×</button>
            </div>
            {selectedTable ? (
              <div className="inspector-selection">
                <strong>{selectedTable}</strong>
                <p>Schema and records can be inspected here when the corresponding backend endpoints are available.</p>
                <button className="secondary-action" onClick={() => setSelectedTable("")}>Clear selection</button>
              </div>
            ) : (
              <div className="panel-empty-state inspector-empty-state">
                <span className="panel-empty-icon">▦</span>
                <strong>No table selected</strong>
                <p>Select a table from Database Explorer to inspect its schema. Records will remain hidden until you explicitly request them.</p>
              </div>
            )}
          </aside>
        )}
      </main>
    </div>
  );
}

export default App;
