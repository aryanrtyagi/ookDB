
import { useState, useEffect, useCallback } from "react";
import { checkDatabaseConnection, executeSQL } from "../services/api";

export function useDatabase() {
  const [isConnected, setIsConnected] = useState(false);
  const [loading, setLoading] = useState(false);
  const [error, setError] = useState("");

  const checkConnection = useCallback(async () => {
    try {
      await checkDatabaseConnection();
      setIsConnected(true);
    } catch {
      setIsConnected(false);
    }
  }, []);

  useEffect(() => {
    checkConnection();
  }, [checkConnection]);

  const runQuery = async (sql) => {
    setLoading(true);
    setError("");

    try {
      const result = await executeSQL(sql);
      return result.data;
    } catch (err) {
      setError(err.message);
      throw err;
    } finally {
      setLoading(false);
    }
  };

  return {
    isConnected,
    loading,
    error,
    runQuery,
    checkConnection,
  };
}
