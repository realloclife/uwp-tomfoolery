#include <memory>

#include "scheduler.hpp"

auto rbx::scheduler::get_jobs() const noexcept -> std::vector<rbx::job> {
	std::vector<rbx::job> jobs{};
	auto current_job = memory::read<uintptr_t>(this->address + offset::job::start);
	auto job_end = memory::read<uintptr_t>(this->address + offset::job::end);
	while (current_job != job_end) {
		jobs.push_back(rbx::job{ current_job });
		current_job += 0x8;
	}
	return jobs;
}
