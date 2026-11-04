/* file.c: Implementation of memory backed file object (mmaped object). */

#include "vm/vm.h"
#include "threads/vaddr.h"
#include "userprog/process.h"

static bool file_backed_swap_in (struct page *page, void *kva);
static bool file_backed_swap_out (struct page *page);
static void file_backed_destroy (struct page *page);

/* DO NOT MODIFY this struct */
static const struct page_operations file_ops = {
	.swap_in = file_backed_swap_in,
	.swap_out = file_backed_swap_out,
	.destroy = file_backed_destroy,
	.type = VM_FILE,
};

/* The initializer of file vm */
void
vm_file_init (void) {
}

/* Initialize the file backed page */
bool
file_backed_initializer (struct page *page, enum vm_type type, void *kva) {
	/* Set up the handler */
	page->operations = &file_ops;

	struct file_page *file_page = &page->file;
}

/* Swap in the page by read contents from the file. */
static bool
file_backed_swap_in (struct page *page, void *kva) {
	struct file_page *file_page UNUSED = &page->file;
}

/* Swap out the page by writeback contents to the file. */
static bool
file_backed_swap_out (struct page *page) {
	struct file_page *file_page UNUSED = &page->file;
}

/* Destory the file backed page. PAGE will be freed by the caller. */
static void
file_backed_destroy (struct page *page) {
	struct file_page *file_page UNUSED = &page->file;
}

static bool
mmap_load_segment (struct page *page, void *aux) {
	struct lazy_load_arg *lazy_load_arg = aux;
	
	file_seek(lazy_load_arg -> file, lazy_load_arg -> ofs);

	if(file_read(lazy_load_arg -> file, page ->frame -> kva, lazy_load_arg ->read_bytes )!=(int) lazy_load_arg->read_bytes){
		// free(lazy_load_arg);
		// palloc_free_page(page->frame->kva);
		return false;
	}
	memset (page->frame->kva + lazy_load_arg->read_bytes, 0, lazy_load_arg->zero_bytes);
	return true;
}

/* Do the mmap */
void *
do_mmap (uintptr_t va, size_t length, int writable,
		struct file *file, off_t offset) {
	
	off_t file_size=file_length(file) - offset;
	size_t read_bytes = file_size < length ? file_size : length;

	for (uintptr_t start = va; start < va+length; start += PGSIZE) {
		size_t page_read_bytes = read_bytes < PGSIZE ? read_bytes : PGSIZE;
		size_t page_zero_bytes = PGSIZE - page_read_bytes;

		/* TODO: Set up aux to pass information to the lazy_load_segment. */
		struct lazy_load_arg *lazy_load_arg=malloc(sizeof(struct lazy_load_arg));                                                                                                                   
		lazy_load_arg->file=file;
		lazy_load_arg->ofs=offset;
		lazy_load_arg->read_bytes=page_read_bytes;
		lazy_load_arg->zero_bytes=page_zero_bytes;
		lazy_load_arg->writable=writable;

		if (!vm_alloc_page_with_initializer (VM_FILE, (void *)va,
					writable, mmap_load_segment, lazy_load_arg))
			return NULL;

		/* Advance. */
		read_bytes -= page_read_bytes;
		offset += PGSIZE;
	}
	
	return va;
		
}

/* Do the munmap */
void
do_munmap (void *addr) {

}
