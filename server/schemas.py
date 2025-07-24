from pydantic import BaseModel
from datetime import date, time, datetime
from typing import Optional

class UserCreate(BaseModel):
    username: str
    password: str

class UserLogin(BaseModel):
    username: str
    password: str

class Token(BaseModel):
    access_token: str
    token_type: str

class TokenData(BaseModel):
    username: Optional[str] = None

class TaskBase(BaseModel):
    text: str
    date: Optional[date] = None
    time: Optional[time] = None
    tag: Optional[str] = None
    completed: bool = False

class TaskCreate(TaskBase):
    pass

class TaskOut(TaskBase):
    id: int
    user_id: int
    created_at: datetime

    model_config = {
        "from_attributes": True
    }

class NoteCreate(BaseModel):
    content: str

class NoteOut(BaseModel):
    id: int
    content: str
    created_at: datetime
    user_id: Optional[int]

    model_config = {
        "from_attributes": True
    }

