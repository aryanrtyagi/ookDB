
from rest_framework.decorators import api_view
from rest_framework.response import Response
from rest_framework import status

from .database import execute_query
from .serializers import SQLQuerySerializer


@api_view(["GET"])
def health_check(request):
    return Response({
        "success": True,
        "data": {
            "status": "ok",
            "engine": "AryDB",
            "message": "AryDB API is running"
        },
        "error": None
    })


@api_view(["POST"])
def execute_sql(request):

    # 1. Validate incoming request
    serializer = SQLQuerySerializer(data=request.data)

    if not serializer.is_valid():
        return Response(
            {
                "success": False,
                "data": None,
                "error": {
                    "message": "Invalid request",
                    "details": serializer.errors
                }
            },
            status=status.HTTP_400_BAD_REQUEST
        )

    sql = serializer.validated_data["sql"]

    try:
        # 2. Execute query using the C++ engine
        result = execute_query(sql)

        # 3. Handle engine-level failure
        if not result.get("success", False):
            return Response(
                {
                    "success": False,
                    "data": None,
                    "error": {
                        "message": result.get(
                            "message", "Query execution failed."
                        )
                    }
                },
                status=status.HTTP_400_BAD_REQUEST
            )

        # 4. Return successful result
        return Response(
            {
                "success": True,
                "data": result,
                "error": None
            },
            status=status.HTTP_200_OK
        )

    except Exception:
        # Log the actual exception on the server.
        import logging
        logger = logging.getLogger(__name__)
        logger.exception("AryDB query execution failed")

        return Response(
            {
                "success": False,
                "data": None,
                "error": {
                    "message": "An internal database error occurred."
                }
            },
            status=status.HTTP_500_INTERNAL_SERVER_ERROR
        )
