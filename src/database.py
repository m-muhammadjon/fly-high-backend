import mysql.connector

DB = mysql.connector.connect(
    host="localhost",
    user="root",
    password="root",
    database="fly_high",
)

cursor = DB.cursor()
cursor.execute("SELECT * FROM users")
result = cursor.fetchall()
print(result)
print(type(result))