#include <windows.h>
#include <climits>
#include <algorithm>

constexpr int CELL_SIZE = 100;
constexpr int WIN_WIDTH = 305, WIN_HEIGHT = 305;
char board[3][3] = {
	{' ', ' ', ' '},
	{' ', ' ', ' '},
	{' ', ' ', ' '}
};
bool humanIsX = true;

bool checkWinner(char board[3][3], const char player)
{
	for (int i = 0; i < 3; ++i)
	{
		if (board[i][0] == player && board[i][1] == player && board[i][2] == player) return true;
		if (board[0][i] == player && board[1][i] == player && board[2][i] == player) return true;
	}
	if (board[0][0] == player && board[1][1] == player && board[2][2] == player) return true;
	if (board[0][2] == player && board[1][1] == player && board[2][0] == player) return true;

	return false;
}

int minimax(char board[3][3], bool isMax)
{
	if (checkWinner(board, 'O')) return 10;
	if (checkWinner(board, 'X')) return -10;

	bool movesLeft = false;
	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++)
			if (board[i][j] == ' ')
				movesLeft = true;

	if (!movesLeft) return 0; // Tie

	if (isMax)
	{
		int best = INT_MIN;
		for (int i = 0; i < 3; i++)
		{
			for (int j = 0; j < 3; j++)
			{
				if (board[i][j] == ' ')
				{
					board[i][j] = 'O';
					best = max(best, minimax(board, !isMax));
					board[i][j] = ' ';
				}
			}
		}
		return best;
	}
	int best = INT_MAX;
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			if (board[i][j] == ' ')
			{
				board[i][j] = 'X';
				best = min(best, minimax(board, !isMax));
				board[i][j] = ' ';
			}
		}
	}
	return best;
}

void bestMove(char board[3][3])
{
	int bestValue = INT_MIN;
	int bestRow = -1;
	int bestCol = -1;

	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			if (board[i][j] == ' ')
			{
				board[i][j] = 'O';
				const int moveValue = minimax(board, false);
				board[i][j] = ' ';
				if (moveValue > bestValue)
				{
					bestRow = i;
					bestCol = j;
					bestValue = moveValue;
				}
			}
		}
	}

	board[bestRow][bestCol] = 'O';
}

void playMoveAndCheckWin(const HWND hwnd, const char player)
{
	InvalidateRect(hwnd, nullptr, TRUE);
	UpdateWindow(hwnd);

	if (checkWinner(board, player))
	{
		MessageBox(hwnd, (player == 'X') ? L"Player X wins!" : L"Player O wins!", L"Winner!",
		           MB_OK | MB_ICONINFORMATION);
		PostQuitMessage(0);
	}
}


void DrawBoard(const HDC hdc)
{
	const HFONT hFont = CreateFont(80, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_TT_PRECIS,
	                               CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Arial");
	SelectObject(hdc, hFont);
	for (int i = 1; i < 3; ++i)
	{
		MoveToEx(hdc, i * CELL_SIZE, 0, nullptr);
		LineTo(hdc, i * CELL_SIZE, 3 * CELL_SIZE);
		MoveToEx(hdc, 0, i * CELL_SIZE, nullptr);
		LineTo(hdc, 3 * CELL_SIZE, i * CELL_SIZE);
	}
	for (int i = 0; i < 3; ++i)
	{
		for (int j = 0; j < 3; ++j)
		{
			RECT cell = {j * CELL_SIZE, i * CELL_SIZE, (j + 1) * CELL_SIZE, (i + 1) * CELL_SIZE};
			if (board[i][j] == 'X')
			{
				DrawTextW(hdc, L"X", 1, &cell, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
			}
			else if (board[i][j] == 'O')
			{
				DrawTextW(hdc, L"O", 1, &cell, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
			}
		}
	}
	DeleteObject(hFont);
}

LRESULT CALLBACK WindowProc(const HWND hwnd, const UINT uMsg, const WPARAM wParam, const LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			const HDC hdc = BeginPaint(hwnd, &ps);
			DrawBoard(hdc);
			EndPaint(hwnd, &ps);
			return 0;
		}
	case WM_LBUTTONDOWN:
		{
			const int xPos = LOWORD(lParam);
			const int yPos = HIWORD(lParam);
			const int row = yPos / CELL_SIZE;
			const int col = xPos / CELL_SIZE;

			const char humanChar = humanIsX ? 'X' : 'O';
			const char aiChar = humanIsX ? 'O' : 'X';

			if (board[row][col] == ' ')
			{
				board[row][col] = humanChar;
				playMoveAndCheckWin(hwnd, humanChar);

				bestMove(board);
				playMoveAndCheckWin(hwnd, aiChar);
			}
			return 0;
		}
	case WM_SETCURSOR:
		{
			SetCursor(LoadCursor(nullptr, IDC_ARROW));
			return TRUE;
		}
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}


int WINAPI wWinMain(const HINSTANCE hInstance, HINSTANCE, PWSTR pCmdLine, const int nCmdShow)
{
	constexpr wchar_t className[] = L"TicTacToe";
	WNDCLASS wc = {};
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = className;
	RegisterClass(&wc);
	RECT rect = {0, 0, WIN_WIDTH, WIN_HEIGHT};
	AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);
	const HWND hwnd = CreateWindowExW(0, className, L"Tic-Tac-Toe", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
	                                  rect.right - rect.left, rect.bottom - rect.top, nullptr, nullptr, hInstance,
	                                  nullptr);
	if (hwnd == nullptr) return 0;
	ShowWindow(hwnd, nCmdShow);
	MSG msg = {};
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return 0;
}
