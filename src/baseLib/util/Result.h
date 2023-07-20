#pragma once

#define RETURN_IF_FALSE(check) if (!check) {return;};
#define RESULT_WHEN_ERR_RETURN(result) if (result.status == ERROR) {return;};

enum RESULT {
    OK,
    ERROR
};

template<class T>
struct Result {
    T _;
    RESULT status;


    static Result Error() {
      return Result<T> {
        ._ = T(),
        .status = ERROR
      };
    }

    static Result Ok(T &value) {
      return Result<T> {
        ._ = value,
        .status = OK
      };
    }
};
