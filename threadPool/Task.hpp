#pragma once
#include <iostream>
#include <string>

class Task
{
public:
    Task()
    {
    }
    Task(int x, int y, char op) : _x(x), _y(y), _op(op), _result(0), _exitCode(0)
    {
    }
    void operator()()
    {
        switch (_op)
        {
        case '+':
            _result = _x + _y;
            break;
        case '-':
            _result = _x - _y;
            break;
        case '*':
            _result = _x * _y;
            break;
        case '/':
        {
            if (_y == 0)
                _exitCode = -1;
            else
                _result = _x / _y;
        }
        break;
        case '%':
        {
            if (_y == 0)
                _exitCode = -2;
            else
                _result = _x % _y;
        }
        break;
        default:
            break;
        }
    }
    std::string formatArg()
    {
        return std::to_string(_x) + _op + std::to_string(_y) + "=";
    }
    std::string formatRes()
    {
        return std::to_string(_result) + "(" + std::to_string(_exitCode) + ")";
    }
    ~Task()
    {
    }

private:
    int _x;
    int _y;
    char _op;

    int _result;
    int _exitCode;
};