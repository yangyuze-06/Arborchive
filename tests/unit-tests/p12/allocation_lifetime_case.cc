#include <cstddef>
#include <new>

struct PlainObject {
  int value;
};

struct ClassAllocated {
  static void *operator new(std::size_t);
  static void operator delete(void *, std::size_t) noexcept;
  int value;
};

struct SizedDeleted {
  static void operator delete(void *, std::size_t) noexcept;
  int value;
};

struct alignas(64) AlignedDeleted {
  static void operator delete(void *, std::align_val_t) noexcept;
  int value;
};

struct alignas(64) SizedAlignedDeleted {
  static void operator delete(void *, std::size_t,
                              std::align_val_t) noexcept;
  int value;
};

struct DestroyingDeleted {
  static void operator delete(DestroyingDeleted *,
                              std::destroying_delete_t) noexcept;
  ~DestroyingDeleted();
};

struct VirtualDeleted {
  static void operator delete(void *) noexcept;
  static void operator delete[](void *) noexcept;
  virtual ~VirtualDeleted();
};

struct FinalVirtualDeleted final {
  static void operator delete(void *) noexcept;
  virtual ~FinalVirtualDeleted();
};

template <class T>
T *dependent_new_array(int extent) {
  return new T[extent];
}

template <class T>
void dependent_delete(T *pointer) {
  delete pointer;
}

void p12_allocation_lifetime(
    int extent, void *placement, int *plain_pointer, int *array_pointer,
    SizedDeleted *sized_pointer, AlignedDeleted *aligned_pointer,
    SizedAlignedDeleted *sized_aligned_pointer,
    DestroyingDeleted *destroying_pointer, VirtualDeleted *virtual_pointer) {
  auto *plain_new = new PlainObject;
  auto *initialized_new = new int(7);
  auto *constant_array = new int[3];
  auto *dynamic_array = new int[extent];
  auto *nested_array = new int[extent][4];
  auto *placement_new = new (placement) int(9);
  auto *over_aligned_new = new AlignedDeleted;
  auto *class_new = new ClassAllocated;

  delete plain_pointer;
  delete[] array_pointer;
  delete sized_pointer;
  delete aligned_pointer;
  delete sized_aligned_pointer;
  delete destroying_pointer;
  delete virtual_pointer;

  VirtualDeleted *global_virtual_pointer = virtual_pointer;
  VirtualDeleted *virtual_array_pointer = virtual_pointer;
  FinalVirtualDeleted *final_virtual_pointer = nullptr;
  ::delete global_virtual_pointer;
  delete[] virtual_array_pointer;
  delete final_virtual_pointer;

  (void)plain_new;
  (void)initialized_new;
  (void)constant_array;
  (void)dynamic_array;
  (void)nested_array;
  (void)placement_new;
  (void)over_aligned_new;
  (void)class_new;
}
