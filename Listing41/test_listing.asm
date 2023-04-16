bits 16
ADD bx, [bx + si]
ADD bx, [bp]
ADD  si, 2
ADD  bp, 2
ADD  cx, 8
ADD bx, [bp]
ADD cx, [bx + 2]
ADD bh, [bp + si + 4]
ADD di, [bp + di + 6]
ADD [bx + si], bx
ADD [bp], bx
ADD [bp], bx
ADD [bx + 2], cx
ADD [bp + si + 4], bh
ADD [bp + di + 6], di
ADD byte [bx], 34
ADD word [bp + si + 1000], 29
ADD ax, [bp]
ADD al, [bx + si]
ADD ax, bx
ADD al, ah
ADD ax, 1000
ADD al, 226
ADD al, 9
SUB bx, [bx + si]
SUB bx, [bp]
SUB  si, 2
SUB  bp, 2
SUB  cx, 8
SUB bx, [bp]
SUB cx, [bx + 2]
SUB bh, [bp + si + 4]
SUB di, [bp + di + 6]
SUB [bx + si], bx
SUB [bp], bx
SUB [bp], bx
SUB [bx + 2], cx
SUB [bp + si + 4], bh
SUB [bp + di + 6], di
SUB byte [bx], 34
SUB word [bx + di], 29
SUB ax, [bp]
SUB al, [bx + si]
SUB ax, bx
SUB al, ah
SUB ax, 1000
SUB al, 226
SUB al, 9
CMP bx, [bx + si]
CMP bx, [bp]
CMP  si, 2
CMP  bp, 2
CMP  cx, 8
CMP bx, [bp]
CMP cx, [bx + 2]
CMP bh, [bp + si + 4]
CMP di, [bp + di + 6]
CMP [bx + si], bx
CMP [bp], bx
CMP [bp], bx
CMP [bx + 2], cx
CMP [bp + si + 4], bh
CMP [bp + di + 6], di
CMP byte [bx], 34
CMP word [4834], 29
CMP ax, [bp]
CMP al, [bx + si]
CMP ax, bx
CMP al, ah
CMP ax, 1000
CMP al, 226
CMP al, 9