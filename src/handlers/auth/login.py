from src.database import cursor
from src.utils import get_token




async def handle_login_user(details):
    """
    Handles the login request from the client.
    """
    email = details.get("email")
    password = details.get("password")
    column_names = ["user_id", "email", "last_name", "first_name", "phone_number"]
    cursor.execute(
        f"SELECT {','.join(column_names)} FROM users WHERE email = '{email}' AND password = '{password}'",
    )
    user = cursor.fetchone()

    if user:
        user_dict = dict(zip(column_names, user))
        token = await get_token(user_dict.get("user_id"))
        return {"status": "OK", "result": {"token": token}}
    return {"status": "ERROR", "result": {"message": "Invalid credentials"}}


