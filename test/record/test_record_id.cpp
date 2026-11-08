#include "../../src/record/record_id.h"
#include <iostream>

int main()
{
    RecordId oldRid(5, 2, 1);
    RecordId newRid(5, 2, 2);

    std::cout << "Old RID valid: "
              << oldRid.isValid() << '\n';

    std::cout << "Same RID: "
              << (oldRid == newRid) << '\n';

    return 0;
}