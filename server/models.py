from sqlalchemy import Column, Integer, String, Boolean, Date, Time, ForeignKey
from database import Base

class User(Base):   
    __tablename__ = "users"
    id = Column(Integer, primary_key = True, index = True)
    username = Column(String, unique = True)
    password_hash = Column(String)
    group_id = Column(String, ForeignKey("groups.id"), nullable = True)

class Group(Base):
    __tablename__ = "groups"
    id = Column(String, primary_key = True)
    name = Column(String)

class Task(Base): 
    __tablename__ = "tasks"
    id = Column(Integer, primary_key = True, index = True)
    user_id = Column(Integer, nullable = True) #null в персональной бд
    text = Column(String)
    date = Column(Date, nullable = True)
    time = Column(Time, nullable = True)
    tag = Column(String)
    completed = Column(Boolean, default = False)

class Note(Base): 
    __tablename__ = "notes"
    id = Column(Integer, primary_key = True)
    user_id = Column(Integer, nullable = True)
    content = Column(String)
    
