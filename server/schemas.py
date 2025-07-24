from pydantic import BaseModel
from typing import Optional 
from datetime import date, time

class UserCreate(BaseModel):
    username: str
    password: str

class UserLogin(BaseModel): 
    username: str
    password: str

class TaskCreate(BaseModel): 
    text: str
    date: Optional[date]
    time: Optional[time]
    tag: Optional[str]
    completed: bool = False

class TaskOut(TaskCreate):
    id: int

class Config:
    orm_mode = True

class GroupCreate(BaseModel):
    name:str

    