from src.database import cursor
from src.utils import get_user_id
from datetime import date


async def handle_get_profile(authorization):
    """
    Handles the get profile request from the client.
    """
    user_id = await get_user_id(authorization)
    column_names = [
        "user_id",
        "email",
        "last_name",
        "first_name",
        "phone_number",
        "birthdate",
        "passport",
        "citizenship",
        "address_country",
        "address_city",
        "address_state",
        "address_line_1",
        "user_state",
    ]
    cursor.execute(
        f"SELECT {','.join(column_names)} FROM users WHERE user_id = {user_id}",
    )
    user = cursor.fetchone()
    print(f"{user=}")
    if user:
        user_dict = dict(zip(column_names, user))
        if isinstance(user_dict["birthdate"], date):
            user_dict["birthdate"] = user_dict["birthdate"].strftime("%Y-%m-%d")
        return {"status": "OK", "result": user_dict}
    return {"status": "ERROR", "result": {"message": "Invalid user"}}
