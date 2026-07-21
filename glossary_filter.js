document.addEventListener('DOMContentLoaded', () => {

    const filterButtonsContainer = document.getElementById('glossary-filter-buttons');
    if (!filterButtonsContainer) {
        return;
    }

    const buttons = filterButtonsContainer.querySelectorAll('.filter-btn');
    const sections = document.querySelectorAll('h2[data-tags]');

    if (buttons.length === 0 || sections.length === 0) {
        return;
    }

    function updateFilter() {

        const activeButtons = filterButtonsContainer.querySelectorAll('.filter-btn.active');
        const activeTags = [];
        activeButtons.forEach(btn => {
            activeTags.push(btn.dataset.tag);
        });

        const isFilterActive = activeTags.length > 0;

        sections.forEach(header => {
            const sectionTag = header.dataset.tags;

            const isVisible = !isFilterActive || activeTags.includes(sectionTag);

            header.style.display = isVisible ? 'block' : 'none';

            let prevElement = header.previousElementSibling;
            if (prevElement && prevElement.tagName === 'HR') {
                prevElement.style.display = isVisible ? 'block' : 'none';
            }

            let nextElement = header.nextElementSibling;
            while (nextElement) {
                if (nextElement.tagName === 'H2' || nextElement.tagName === 'HR') {
                    break;
                }
                nextElement.style.display = isVisible ? 'block' : 'none';
                nextElement = nextElement.nextElementSibling;
            }
        });
    }

    buttons.forEach(btn => {
        btn.addEventListener('click', () => {

            btn.classList.toggle('active');

            updateFilter();
        });
    });

    updateFilter();

});