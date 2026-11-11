from django.urls import path
from . import views

urlpatterns = [
    path("health/", views.health_check, name="health_check"),
    path("query/", views.execute_sql, name="execute_sql"),
]
