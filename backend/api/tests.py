from unittest.mock import patch

from django.test import SimpleTestCase
from rest_framework.test import APITestCase


class HealthCheckTests(APITestCase):

    def test_health_endpoint(self):
        response = self.client.get("/api/health/")

        self.assertEqual(response.status_code, 200)
        self.assertTrue(response.data["success"])
        self.assertEqual(response.data["data"]["engine"], "AryDB")


class SQLQueryTests(APITestCase):

    @patch("api.views.execute_query")
    def test_successful_select(self, mock_execute):
        mock_execute.return_value = {
            "columns": ["id", "name"],
            "rows": [[1, "AryDB"]],
            "affected_rows": 0,
            "message": "Full table scan",
            "success": True
        }

        response = self.client.post(
            "/api/query/",
            {"sql": "SELECT * FROM api_demo;"},
            format="json"
        )

        self.assertEqual(response.status_code, 200)
        self.assertTrue(response.data["success"])
        self.assertEqual(
            response.data["data"]["rows"],
            [[1, "AryDB"]]
        )

    def test_empty_sql_rejected(self):
        response = self.client.post(
            "/api/query/",
            {"sql": ""},
            format="json"
        )

        self.assertEqual(response.status_code, 400)
        self.assertFalse(response.data["success"])

    def test_missing_sql_rejected(self):
        response = self.client.post(
            "/api/query/",
            {},
            format="json"
        )

        self.assertEqual(response.status_code, 400)

    def test_get_method_not_allowed(self):
        response = self.client.get("/api/query/")

        self.assertEqual(response.status_code, 405)

    @patch("api.views.execute_query")
    def test_engine_failure(self, mock_execute):
        mock_execute.return_value = {
            "columns": [],
            "rows": [],
            "affected_rows": 0,
            "message": "Invalid SQL syntax",
            "success": False
        }

        response = self.client.post(
            "/api/query/",
            {"sql": "INVALID SQL"},
            format="json"
        )

        self.assertEqual(response.status_code, 400)
        self.assertFalse(response.data["success"])
        self.assertIsNone(response.data["data"])
