# ChatServer
MFC 채팅 서버 프로그램입니다. 데이터 베이스는 MSSQL을 사용하였습니다.

채팅 내역을 저장하는 테이블 구조입니다. 
```sql
create table chat
(
    CHAT_SEQ     int identity
        constraint chat_pk
            primary key nonclustered,
    CHAT_CONTENT varchar(200),
    IP           varchar(20),
    REG_TIME     datetime
)
go

exec sp_addextendedproperty 'MS_Description', '채팅', 'SCHEMA', 'chat', 'TABLE', 'chat'
go

exec sp_addextendedproperty 'MS_Description', '채팅내역 SEQ', 'SCHEMA', 'chat', 'TABLE', 'chat', 'COLUMN', 'CHAT_SEQ'
go

exec sp_addextendedproperty 'MS_Description', '채팅 내용', 'SCHEMA', 'chat', 'TABLE', 'chat', 'COLUMN', 'CHAT_CONTENT'
go

exec sp_addextendedproperty 'MS_Description', 'IP 주소', 'SCHEMA', 'chat', 'TABLE', 'chat', 'COLUMN', 'IP'
go

exec sp_addextendedproperty 'MS_Description', '입력시간', 'SCHEMA', 'chat', 'TABLE', 'chat', 'COLUMN', 'REG_TIME'
go
```
접속 내역을 저장하는 테이블 구조입니다.
```sql
create table access_list
(
    ACCESSLIST_SEQ int identity
        constraint access_list_pk
            primary key nonclustered,
    IP             varchar(20),
    TYPE           char,
    REG_TIME       datetime
)
go

exec sp_addextendedproperty 'MS_Description', '접속 목록', 'SCHEMA', 'chat', 'TABLE', 'access_list'
go

exec sp_addextendedproperty 'MS_Description', '접속 목록 SEQ', 'SCHEMA', 'chat', 'TABLE', 'access_list', 'COLUMN',
     'ACCESSLIST_SEQ'
go

exec sp_addextendedproperty 'MS_Description', 'IP 주소', 'SCHEMA', 'chat', 'TABLE', 'access_list', 'COLUMN', 'IP'
go

exec sp_addextendedproperty 'MS_Description', '접속 여부(1: 접속, 2: 접속 해제)', 'SCHEMA', 'chat', 'TABLE', 'access_list',
     'COLUMN', 'TYPE'
go

exec sp_addextendedproperty 'MS_Description', '입력 시간', 'SCHEMA', 'chat', 'TABLE', 'access_list', 'COLUMN', 'REG_TIME'
go
```
