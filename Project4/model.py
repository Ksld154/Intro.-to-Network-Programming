from peewee import *  # pylint: disable=W0614


db = SqliteDatabase('Project4.db', pragmas={'foreign_keys': 1})


class BaseModel(Model):
    class Meta:
        database = db


class User(BaseModel):
    username = CharField(unique=True)
    password = CharField()


class Invitation(BaseModel):
    inviter = ForeignKeyField(User, on_delete='CASCADE')
    invitee = ForeignKeyField(User, on_delete='CASCADE')


class Friend(BaseModel):
    user   = ForeignKeyField(User, on_delete='CASCADE')
    friend = ForeignKeyField(User, on_delete='CASCADE')


class Post(BaseModel):
    user = ForeignKeyField(User, on_delete='CASCADE')
    message = CharField()


class Follow(BaseModel):
    follower = ForeignKeyField(User, on_delete='CASCADE')
    followee = ForeignKeyField(User, on_delete='CASCADE')


class Token(BaseModel):
    token = CharField(unique=True)
    owner = ForeignKeyField(User, on_delete='CASCADE')

class Group(BaseModel):
    group_name = CharField(unique=True)

class Subscribe(BaseModel):
    user  = ForeignKeyField(User,  on_delete='CASCADE')
    group = ForeignKeyField(Group, on_delete='CASCADE')

if __name__ == '__main__':
    db.connect()
    db.create_tables([User, Invitation, Friend, Post, Follow, Token, Group, Subscribe])
