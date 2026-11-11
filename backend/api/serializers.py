from rest_framework import serializers

class SQLQuerySerializer(serializers.Serializer):
    sql = serializers.CharField(
        required=True,
        allow_blank=False,
        trim_whitespace=True,
        max_length=10000
    )

    def validate_sql(self, value):
        if not value.strip():
            raise serializers.ValidationError(
                "SQL query cannot be empty."
            )

        return value
