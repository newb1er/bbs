# bbs

A simple BBS (Bulletin Board System) implementation.

# Compile

Clone this repo and enter below command to compile.

```sh
make
```

If you want to clear up every binary built before, enter

```sh
make clean
```

# Test it

First, spin up the server.

```sh
./bbs [port]

---
port: the port the server listen to, default is 2233
```

Then, use any tcp communication tool (e.g. `nc`) to connect.

```sh
nc localhost <server-port>
```

# Documentation

## Commands

### `register`

Register a new user entry.

#### Usage

```sh
register <username> <password>
```

### `login`

Login with the identity created by `register` command.

#### Usage

```sh
login <username> <password>
```

### `logout`

Logout the user.

#### Usage

```sh
logout
```

### `exit`

Terminate the session.

#### Usage

```sh
exit
```

### `create-board`

Create a new board.

#### Usage

```sh
create-board <board-name>
```

### `create-post`

Create a new post to a board.

#### Usage

```sh
create-post <board-name> --title <title> --content <content>
```

### `list-board`

List every board in the system.

#### Usage

```sh
list-board
```

### `list-post`

List every post on a board.

#### Usage

```sh
list-post <board-name>
```

### `read`

Read a post.

#### Usage

```sh
read <post-S/N>
```

(S/N can be obtained by `list-post`)

### `delete-post`

Delete the specified post.

#### Usage

```sh
delete-post <post-S/N>
```

(S/N can be obtained by `list-post`)

### `update-post`

Change the title or content of a post.

#### Usage

```sh
update-post <post-S/N> --title/content <text>
```

### `comment`

Leave a comment to a post.

#### Usage

```sh
comment <post-S/N> <comment>
```
