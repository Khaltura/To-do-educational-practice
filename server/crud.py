from sqlalchemy.orm import Session
import models, schemas

def get_user_by_username(db: Session, username: str):
    return db.query(models.User).filter(models.User.username == username).first()

def create_user(db: Session, username: str, password_hash: str):
    user = models.User(username=username, password_hash=password_hash)
    db.add(user)
    db.commit()
    db.refresh(user)
    return user

def create_task(db: Session, task: schemas.TaskCreate, user_id: int):
    db_task = models.Task(**task.dict(), user_id=user_id)
    db.add(db_task)
    db.commit()
    db.refresh(db_task)
    return db_task

def get_tasks(db: Session, user_id: int):
    return db.query(models.Task).filter(models.Task.user_id == user_id).order_by(models.Task.date, models.Task.time).all()

def delete_task(db: Session, task_id: int, user_id: int):
    task = db.query(models.Task).filter_by(id=task_id, user_id=user_id).first()
    if task:
        db.delete(task)
        db.commit()
        return True
    return False

def update_task(db: Session, task_id: int, user_id: int, updates: dict):
    task = db.query(models.Task).filter_by(id=task_id, user_id=user_id).first()
    if not task:
        return None
    for key, value in updates.items():
        setattr(task, key, value)
    db.commit()
    db.refresh(task)
    return task

def create_note(db: Session, note: schemas.NoteCreate, user_id: int):
    db_note = models.Note(content=note.content, user_id=user_id)
    db.add(db_note)
    db.commit()
    db.refresh(db_note)
    return db_note

def get_notes(db: Session, user_id: int):
    return db.query(models.Note).filter(models.Note.user_id == user_id).order_by(models.Note.created_at.desc()).all()

def delete_note(db: Session, note_id: int, user_id: int):
    note = db.query(models.Note).filter_by(id=note_id, user_id=user_id).first()
    if note:
        db.delete(note)
        db.commit()
        return True
    return False

def update_note(db: Session, note_id: int, user_id: int, content: str):
    note = db.query(models.Note).filter_by(id=note_id, user_id=user_id).first()
    if note:
        note.content = content
        db.commit()
        db.refresh(note)
        return note
    return None

