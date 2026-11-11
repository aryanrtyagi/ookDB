function QueryHistory({ history, onSelect, onClear }) {
  return (
    <div className="history-panel">
      <div className="history-header">
        <div>
          <h3>Query History</h3>
          <p>Previously executed SQL queries</p>
        </div>

        <button
          className="clear-history"
          onClick={onClear}
          disabled={history.length === 0}
        >
          Clear History
        </button>
      </div>

      {history.length === 0 ? (
        <div className="history-empty">
          <h3>No query history</h3>
          <p>Your executed queries will appear here.</p>
        </div>
      ) : (
        <div className="history-list">
          {history.map((item) => (
            <button
              className="history-item"
              key={item.id}
              onClick={() => onSelect(item.sql)}
            >
              <div className="history-item-top">
                <span className="history-sql">
                  {item.sql.replace(/\s+/g, " ").trim()}
                </span>

                <span
                  className={`history-status ${
                    item.success ? "success" : "failed"
                  }`}
                >
                  {item.success ? "Success" : "Failed"}
                </span>
              </div>

              <span className="history-time">
                {item.timestamp}
              </span>
            </button>
          ))}
        </div>
      )}
    </div>
  );
}

export default QueryHistory;
