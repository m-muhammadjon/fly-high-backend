import jwt
from src.settings import Settings

async def get_token(user_id: int) -> str:
    """
    Get the token for the user.
    """
    return jwt.encode({"user_id": user_id}, Settings.SECRET, algorithm="HS256")
